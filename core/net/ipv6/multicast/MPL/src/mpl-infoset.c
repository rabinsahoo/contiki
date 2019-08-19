#include "mpl-information.h"
#include "mpl-util.h"
#include "mpl-ctrlmsg.h"
#include "mpl-trickle.h"
#include "mpl-errno.h"
#include "mpl-datamsg.h"

#define MPL_BUFFERED_SET_BITVECTOR \
	(MPL_BUFFERED_MESSAGE_SET_CB_SIZE / sizeof(MPL_UCHAR)) + 1

#define MPL_SEED_SET_BITVECTOR (MPL_SEED_SET_SIZE/sizeof(MPL_UCHAR)) + 1

/* We are keeping both seed set CB and Buffered message set CB common
across the Multiple Domain and Multiple Seeds respectively *
This can be per MPL domain and Per Seed entry if dynamic memory allocation
is supported */
MplSeedSetS g_MplSeedSetArray[MPL_SEED_SET_SIZE];
MPL_UCHAR g_SeedSetBitVector[MPL_SEED_SET_BITVECTOR] = {0};
MPL_UCHAR g_SeedSetScanVector[MPL_SEED_SET_BITVECTOR] = {0};


MplBuffferedMessageS g_BufferedMsgArray[MPL_BUFFERED_MESSAGE_SET_CB_SIZE];
MPL_UCHAR g_BuffSetBitVector[MPL_BUFFERED_SET_BITVECTOR] = {0};

MplPeriodicTimer g_MplCtrlTimer;
extern MplExternalInterfaceS g_ExternalInterfaceApi;

MplBuffferedMessageS *MplInfoSetAllocNewBufferSetEntry(void)
{
	int freeIndex;
	
	freeIndex = MplUtilGetFreeMemoryIndex(g_BuffSetBitVector, 
				MPL_BUFFERED_MESSAGE_SET_CB_SIZE);
	if (freeIndex < 0){
		printf("No Free CB exists Seed set need to recycle\n");
		return NULL;
	}

	return &g_BufferedMsgArray[freeIndex];
}

void MplInfoSetFreeBufferedMsgEntry
(
	MplBuffferedMessageS *pstEntry
)
{
	MPL_INT index;
	
	index = pstEntry - &(g_BufferedMsgArray[0]);
	MplUtilFreeMemoryIndex(g_BuffSetBitVector, index);
	return;
}

MplSeedSetS *MplInfoSetAllocSeedEntry(void)
{	
	int freeIndex;
		
	freeIndex = MplUtilGetFreeMemoryIndex(g_SeedSetBitVector, 
					MPL_SEED_SET_SIZE);
	if (freeIndex < 0){
		printf("No Free CB exists Seed set need to recycle\n");
		return NULL;
	}
	
	return &(g_MplSeedSetArray[freeIndex]);
}

void MplInfoSetFreeSeedEntry(MplSeedSetS *entry)
{	
	MPL_INT idx = 0;

	idx =( entry - &(g_MplSeedSetArray[0]));

	MplUtilFreeMemoryIndex(g_SeedSetBitVector, idx);
}

MPL_VOID MplInfoSetReleaseSeedSetEntry(MplSeedSetS *pstSeed)
{
	MplBuffferedMessageS *pstCur;
	MplBuffferedMessageS *pstTemp;

	pstCur = pstSeed->pstBufMsgSet;
	while(pstCur){
		/* Stop Timer */
		g_ExternalInterfaceApi.pfStopTimer(&(pstCur->stTrickleTimer));
		pstTemp = pstCur;
		pstCur = pstCur->pstNext;
		MplInfoSetFreeBufferedMsgEntry(pstTemp);
	}

	MplInfoSetFreeSeedEntry(pstSeed);
	return;	
}

 void MplInfoSetAddSeed2SeedSetList
(
	MplDomainS *pstMplDomain,
	MplSeedSetS *pstSeedEntry
)
{
	if (pstMplDomain->pstRemoteSeeds == NULL){
		pstMplDomain->pstRemoteSeeds = pstSeedEntry;
		pstSeedEntry->pstNext = NULL;
	}
	else{
		/* Append to begining */
		pstSeedEntry->pstNext = pstMplDomain->pstRemoteSeeds;
		pstMplDomain->pstRemoteSeeds = pstSeedEntry;
	}

	pstMplDomain->usRSeedsCnt++;
}

MPL_VOID MplInfoSetSeedLifeTimeHandler(void *pvUserData)
{
	/*We Will reduce the life time of  */
	MplDomainS *pstDomain = (MplDomainS *)pvUserData;
	MplSeedSetS *pstCurSeed;
	MplSeedSetS *pstPrevSeed;
	
	if (pstDomain->usRSeedsCnt){
		pstCurSeed = pstDomain->pstRemoteSeeds;
		while(pstCurSeed){
			pstCurSeed->uiRemainLifeTime--;
			pstPrevSeed = pstPrevSeed;
			if (pstCurSeed->uiRemainLifeTime <= 0){
				/* Remove the Seed and free it */
				MplSeedSetS *pstTemp = pstCurSeed;
				pstCurSeed = pstCurSeed->pstNext;
				if (pstDomain->pstRemoteSeeds == pstTemp){
					pstDomain->pstRemoteSeeds = pstCurSeed;
				}
				else{
					pstPrevSeed->pstNext = pstCurSeed;
				}

				MplInfoSetReleaseSeedSetEntry(pstTemp);
			}
		}
	}

	g_ExternalInterfaceApi.pfSetTimer(&(pstDomain->stPeriodicTmr), 1000, 
		MplInfoSetSeedLifeTimeHandler, pstDomain);
}

MPL_VOID MplInfoSetStartMplDomain(MplDomainS *pstDomain)
{
	g_ExternalInterfaceApi.pfSetTimer(&(pstDomain->stPeriodicTmr), 1000, 
		MplInfoSetSeedLifeTimeHandler, pstDomain);
}

MPL_UINT MplCtrlMsgTimeOutHandler(MplTrickleTimerS *pstTmr, void *pvUserData)
{
	MplDomainS *pstDomain;

	pstDomain = (MplDomainS *)pvUserData;

	/* Encode and send the Message */
	if (MplCtrlMsgSend(pstDomain)!=MPL_RET_SUCCESS){
		printf("Failed to send the control message\n");
		return MPL_RET_SUCCESS;
	}

	pstTmr->ucNumTrans = pstTmr->ucNumTrans + 1;
	if (pstTmr->ucNumTrans == MPL_CONTROL_MESSAGE_TIMER_EXPIRATIONS){
		return MPL_RET_STOP_TRICKLE;
	}

	return MPL_RET_SUCCESS;
 }

MPL_UINT MplInfoSetDataMsgTimeOutHandler(MplTrickleTimerS *pstTmr, void *pvUserData)
{
	/* Send the buffered message out */
	if (MplDataMsgTransmit((MplBuffferedMessageS *) pvUserData, NULL) 
		!= MPL_RET_SUCCESS){
		printf("Failed to send the data message\n");
		return MPL_RET_SUCCESS;
	}
	
	pstTmr->ucNumTrans ++;
	if (pstTmr->ucNumTrans == MPL_DATA_MESSAGE_TIMER_EXPIRATIONS){
		return MPL_RET_STOP_TRICKLE;
	}

	return MPL_RET_SUCCESS;
}

MPL_VOID MplInfoSetStartOrResetCtrlMsgTimer
(
	MplDomainS *pstDomain
)
{
#if MPL_CONTROL_MESSAGE_TIMER_EXPIRATIONS > 0
	MplTrickleResetTimer(&(pstDomain->stCtrlMsgTmr), pstDomain);
#endif
}

/* 
	This timer MUST be set if the 
	1- MPL control message doesn't include an MPL seed for the MPL data message
	2- If a particular DATA message not found in the received control vector.
*/

MPL_VOID MplInfoSetStartOrResetBuferMsgTimer
(
	MplBuffferedMessageS *pstBuffMsg
)
{
	
#if MPL_DATA_MESSAGE_TIMER_EXPIRATIONS > 0
	MplTrickleResetTimer(&(pstBuffMsg->stTrickleTimer), pstBuffMsg);
#endif

}

MPL_VOID MplInfoSetFindMissingSeedInPeer(MplDomainS *pstDomain)
{
	MplSeedSetS *pstTmpEntry;
	MPL_INT idx = 0;
	MPL_INT byteloc;
    MPL_INT bitpos;
	MPL_BOOL bNeedStartCtrTrickle = MPL_FALSE;

	pstTmpEntry = pstDomain->pstRemoteSeeds;
	
	while(pstTmpEntry){		
		idx =( pstTmpEntry - &(g_MplSeedSetArray[0]));
		byteloc = (idx/MPL_BITS_INBYTE);
    	bitpos = ((byteloc * MPL_BITS_INBYTE) - idx);
    	bitpos = (MPL_BITS_INBYTE - (bitpos + 1));

		if (!(g_SeedSetScanVector[byteloc] & (MPL_UNIT_VALUE << bitpos))){
			/* The corresponding seed entry not found in the peer so reset the 
			trickle timer of all the buffered messages associated to this mpl 
			seed*/
			MplBuffferedMessageS *pstBuffMsg;

			pstBuffMsg = pstTmpEntry->pstBufMsgSet;
			while(pstBuffMsg){				
				MplInfoSetStartOrResetBuferMsgTimer(pstBuffMsg);
				pstBuffMsg = pstBuffMsg->pstNext;
			}
			
			bNeedStartCtrTrickle = MPL_TRUE;
		}		
	}

	if (bNeedStartCtrTrickle){
		MplInfoSetStartOrResetCtrlMsgTimer(pstDomain);
	}

	return;
}

MPL_VOID MplInfoSetSetSeedScanBit
(
	MplSeedSetS *pstSeed
)
{
	/* Get the Index of the seed entry from  the CB */
	MPL_INT idx = 0;
	MPL_INT byteloc;
    MPL_INT bitpos;
	
	idx =( pstSeed - &(g_MplSeedSetArray[0]));

	/* Now find the bit position in the scann array */
    byteloc = (idx/MPL_BITS_INBYTE);
    bitpos = ((byteloc * MPL_BITS_INBYTE) - idx);
    bitpos = (MPL_BITS_INBYTE - (bitpos + 1));

	g_SeedSetScanVector[byteloc] |= (MPL_UNIT_VALUE << bitpos);
	
}

MplSeedSetS *MplInfoSetGetSeedBySeedID
(
	MplDomainS *pstMplDomain,
	MPL_UCHAR *ucSeedId,
	MPL_UCHAR  ucSeedIdLen
)
{
	MplSeedSetS *pstTemp;

	pstTemp = pstMplDomain->pstRemoteSeeds;
	while(pstTemp)
	{
		if (!memcmp(pstTemp->ucSeedID, ucSeedId, ucSeedIdLen ))
		{
			return pstTemp;
		}

		pstTemp = pstTemp->pstNext;
	}

	return NULL;
}

/*	
If memory is limited, an MPL Forwarder SHOULD reclaim memory resources by:
1- Incrementing MinSequence entries in a Seed Set and deleting MPL  Data 
  Messages in the corresponding Buffered Message Set that fall  below the 
  MinSequence value.

 2- Deleting other Seed Set entries that have expired and the  corresponding 
  MPL Data Messages in the Buffered Message Set.
*/

static void MpplInfoSetCleanBuffsetByMinSequence
(
	MplSeedSetS *pstSeedInfo
)
{
	MplBuffferedMessageS *temp;
	MplBuffferedMessageS *cur;
	MplBuffferedMessageS *prev;
	
	/* Increment the min sequence number */
	pstSeedInfo->ucMinSequence += 1;

	/* Find the Buffered messages whose sequence number is less than the min
	sequence number */
	prev = cur = pstSeedInfo->pstBufMsgSet;
	while(cur)
	{
		if (cur->ucSequence < pstSeedInfo->ucMinSequence){
			/* Free this buffered message entry */
			if (cur == pstSeedInfo->pstBufMsgSet){
				pstSeedInfo->pstBufMsgSet = cur->pstNext;
			}
			else
			{
				prev->pstNext = cur->pstNext;
				temp = cur;
			}
		}

		prev = cur;
		cur = cur->pstNext;
		if (temp)
		{
			/* Free the Buffered message */
			MplInfoSetFreeBufferedMsgEntry(temp);
			temp = NULL;
		}
	}

	return ;
}

static void MplInfoSetFreeAgedSeedsetEntry
(
	MplDomainS *pstMplDomain
)
{
}

 MPL_VOID MplInfoSetReclaimMemory
(
	MplDomainS *pstMplDomain,
	MplSeedSetS *pstSeedSetEntry
)
{
	/*Free the Buffered Message set of seed by incrementing the sequence number */
	MpplInfoSetCleanBuffsetByMinSequence(pstSeedSetEntry);

	/* Free the aged mpl seed set entry */
	MplInfoSetFreeAgedSeedsetEntry(pstMplDomain);
}

MPL_BOOL MplInoSetIsExistBufferedMsgSetEntry
(
	MplSeedSetS *pstSeedSet,
	MPL_UCHAR ucSeqNo
)
{
	MplBuffferedMessageS *temp;

	temp = pstSeedSet->pstBufMsgSet;
	while(temp)
	{
		if (temp->ucSequence == ucSeqNo)
		{
			return MPL_TRUE;
		}
	}

	return MPL_FALSE;
}
 
