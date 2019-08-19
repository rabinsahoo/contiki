#include "mpl-ctrlmsg.h"
#include "mpl-datamsg.h"
#include "mpl-information.h"
#include "mpl-errno.h"

#define MPL_ICMP6_HDR_SIZE 4
#define MPL_IPV6_HDR_SIZE 40
#define MPL_ICMPv6_TYPE 159
#define MPL_ICMPv6_CODE 0
#define MPL_SIDLEN_128 0x03
#define MPL_SIDLEN_64 0x02
#define MPL_SIDLEN_16 0x01

/* Only min-seqno , bm-len and S flag */
#define MPL_SEED_INFO_HDRSIZE 2

/* As bm-len is 6 bit so the length of the bit vector array can be
64 bytes*/
#define MPL_SEED_INFO_BUFFMSG_LEN 64

extern MplExternalInterfaceS g_ExternalInterfaceApi;

MPL_BOOL MplCtrlMsgCheckIsSetBitVector
(
	MPL_UCHAR ucDatamsgIdx,
	MPL_UCHAR *pucBitVector,
	MPL_UCHAR bitVectorLen
)
{
	MPL_UCHAR iByteIdx;
	MPL_UCHAR iBitIndex;

	/*
		Find the Byte Position from  the Index 
		1--> 0 2-->1 3-->2 4-->3 5-->4 6-->5
		7-->6 8--> 7
	*/
	iByteIdx = ucDatamsgIdx / 8;
	iBitIndex = ucDatamsgIdx % 8;

	if (iByteIdx > bitVectorLen){
		return MPL_FALSE;
	}

	if ((pucBitVector[iByteIdx]) & (1 << iBitIndex)){
		return MPL_TRUE;
	}

	return MPL_FALSE;
}

MPL_INT MpllCtrlMsgProcBuffMplMsgsBitVector
(
	MplDomainS *pstDomain,
	MplSeedSetS *pstSeedEntry,
	MPL_UCHAR *pucBuffMsgVector,
	MPL_UCHAR ucVectorlen,
	MPL_UCHAR usMinSeq
)
{
	/* Here we need to process the control message  using two rules
	1-- If the the processing node has to receive some data from sender
	2-- If the processing node has to send some data to the sender
	Section 10.3 RFC 7731
	*/
	MplBuffferedMessageS *pstBuffSetEntry;
	MPL_UINT ByteIndex;
	MPL_UCHAR ucByte;
	MPL_UINT uiLocalMiss = 0;
	MPL_UINT uiRemoteMiss = 0;
	int i;

	/* The MPL Control Message includes an MPL Seed that does not exist
      in the MPL Domain's Seed Set
    */
	
	if (NULL == pstSeedEntry){
		/* Seed entry doesn't exist So we need to reset or start the 
		trickle timer associated to the control message transmission*/
		MplInfoSetStartOrResetCtrlMsgTimer(pstDomain);
		return MPL_RET_DETECTED_INCONSISTENCY;
	}

	MplInfoSetSetSeedScanBit(pstSeedEntry);

	/* Seed found So check the bit vector against the already stored 
	      buffered message set against the seed */
	for (ByteIndex = 0; ByteIndex < (MPL_UINT)ucVectorlen; ByteIndex ++){
		ucByte = pucBuffMsgVector[ByteIndex];
		for (i = 0; i< 8 ; i++){
			if (ucByte & (1 << i)){
				if ((usMinSeq + i) > pstSeedEntry->ucMinSequence){
					uiLocalMiss++;	
				}
			}
			
		}
	}

	/* Scan the seed's buffered message set entry to find out any buffered 
	message that is not yet received by the neighbor */
	pstBuffSetEntry = pstSeedEntry->pstBufMsgSet;
	while(pstBuffSetEntry){
		if (pstBuffSetEntry->ucSequence >= usMinSeq){
			if (!MplCtrlMsgCheckIsSetBitVector(
				(pstBuffSetEntry->ucSequence - usMinSeq), 
				pucBuffMsgVector, ucVectorlen)){
				/* This  entry   neighbor doesn't have */
				uiRemoteMiss++;
				MplInfoSetStartOrResetBuferMsgTimer(pstBuffSetEntry);
			}
		}

		pstBuffSetEntry = pstBuffSetEntry->pstNext;
	}
	
	/* If Any data message is missing then send */
	if (uiRemoteMiss || uiLocalMiss){
		//MplCtrlMsgStartOrResetCtrlMsgTimer(pstDomain);
		return MPL_RET_DETECTED_INCONSISTENCY;
	}
	
	return MPL_RET_SUCCESS;
}

MPL_VOID MplCtrlMsgEncodeBufferedMsgSet
(
	MplSeedSetS *pstMplSeed,
	MPL_UCHAR *pucCtrlMsgBuf,
	MPL_UCHAR *pucBmLen
)
{
	MplBuffferedMessageS *pstCurBufMsg;
	MPL_UCHAR ucSeqDiff;
	MPL_UCHAR  ucBytePos;
	MPL_UCHAR  ucBitPos;
	MPL_UCHAR  ucBmLen = 0;

	pstCurBufMsg = pstMplSeed->pstBufMsgSet;

	while(pstCurBufMsg){
		ucSeqDiff = pstCurBufMsg->ucSequence - pstMplSeed->ucMinSequence;
		ucBytePos = ucSeqDiff / 8;

		if (ucBytePos > ucBmLen){
			ucBmLen = ucBytePos;
		}

		ucBitPos = ucSeqDiff % 8;
		pucCtrlMsgBuf[ucBytePos] |= (1 << ucBitPos);

		pstCurBufMsg = pstCurBufMsg->pstNext;
	}

	*pucBmLen = ucBmLen;
	return;
}

MPL_VOID MplCtrlMsgProcessInput
(
	IN MPL_UCHAR *pcIcmpv6MsgBuf,
	IN MPL_USINT usMsgLen,
	IN Ip6AddrS *pstSrcAddr,
	IN Ip6AddrS *pstDstAddr
)
{
	MplDomainS *pstMplDomain;
	MplSeedSetS *pstSeedEntry;
	MPL_USINT usSeedInfosLen;
	MPL_UCHAR *pucMplCtrlmsgBuf;
	MPL_USINT usMinSeq;
	MPL_UCHAR ucS;
	MPL_USINT usbmlen;
	MPL_UCHAR ucSeedId[MPL_SEEDID_MAX];
	MPL_UCHAR  ucSeedidlen = 0;
	MPL_UCHAR  ucBuffMsgVector[MPL_SEED_INFO_BUFFMSG_LEN];
	MPL_USINT usSeeedInfoCnt = 0;
	MPL_USINT usInConsistencyCnt = 0;
	
	
	/* Check the domain if node is not part of this domain drop the msg*/
	/* The destintion address is the link scoped MPL domain address of
	the corresponding domain */
	pstMplDomain = MplFindDomainByLLDomainId(pstDstAddr);
	if (NULL == pstMplDomain){
		printf("Node is not part of MPL domain drop the msg\n");
		return;
	}

	/* Process the seed */
	usSeedInfosLen = usMsgLen - MPL_ICMP6_HDR_SIZE;
	pucMplCtrlmsgBuf = pcIcmpv6MsgBuf + MPL_ICMP6_HDR_SIZE;
	
	while(usSeedInfosLen){
		if (usSeedInfosLen < MPL_SEED_INFO_HDRSIZE){
			printf("Received a corrupt MPL ctrl msg\n");
		}
		return;

		/* Get the min sequence */
		usMinSeq = (MPL_USINT)(pucMplCtrlmsgBuf[0]);

		/* First 6 bits are the length of the buffered mpl message
		bit vector  11111100 0xFC*/
		usbmlen = (pucMplCtrlmsgBuf[1] & 0xFC);
		ucS = (pucMplCtrlmsgBuf[1] & 0x03);

		usSeedInfosLen -= MPL_SEED_INFO_HDRSIZE;
		pucMplCtrlmsgBuf += MPL_SEED_INFO_HDRSIZE;
		
		/* Find the Seed ID len */
		if(ucS == 1){
			ucSeedidlen = 2;
		}else if (ucS == 2){
			ucSeedidlen = 8;
		}else if(ucS == 3){
			ucSeedidlen = 16;
		}
		
		if ((ucSeedidlen + usbmlen) > usSeedInfosLen ){
			printf("Corrupt MPL control Message\n");
			return;
		}

		if (!ucSeedidlen){
			MPL_MEMCPY(ucSeedId, MPL_SEEDID_MAX, pstSrcAddr, sizeof(Ip6AddrS));
			ucSeedidlen = sizeof(Ip6AddrS);
		}else{
			MPL_MEMCPY(ucSeedId, MPL_SEEDID_MAX, pucMplCtrlmsgBuf, 
				ucSeedidlen);				
		}

		MPL_MEMCPY(ucBuffMsgVector, MPL_SEED_INFO_BUFFMSG_LEN, 
					(pucMplCtrlmsgBuf + ucSeedidlen), usbmlen);
		pstSeedEntry = MplInfoSetGetSeedBySeedID( pstMplDomain, ucSeedId, 
							ucSeedidlen);
		
		pstSeedEntry ? usSeeedInfoCnt ++ : 0;
		if (MpllCtrlMsgProcBuffMplMsgsBitVector(pstMplDomain, pstSeedEntry, 
									ucBuffMsgVector, usbmlen, usMinSeq) == 
									MPL_RET_DETECTED_INCONSISTENCY){
			usInConsistencyCnt++;
		}
		
		pucMplCtrlmsgBuf += (ucSeedidlen + usbmlen);
		usSeedInfosLen -= (ucSeedidlen + usbmlen);
	}

	/* Check if the seed set hits are same as the seedset we have */
	if (pstMplDomain->usRSeedsCnt != usSeeedInfoCnt || usInConsistencyCnt){
		MplInfoSetStartOrResetCtrlMsgTimer(pstMplDomain);
	}
	else{
		/* Consistency Transmission Detected . Increase the consistent counter
		of the control message trickle*/
		pstMplDomain->stCtrlMsgTmr.ucCounter++;		
	}

	return;
}


MPL_INT MplCtrlMsgSend
(
	MplDomainS *pstDomain
)
{
	MPL_UCHAR ucCtrlMsgBuf[MPL_CTRL_MSG_MAX_SIZE] = {0};
	MplSeedSetS *pstCurSeed;
	MPL_SINT curBufIdx = 0;
	MPL_SINT bmlenIdx = 0;
	MPL_UCHAR ucBmlen = 0;

	/* We will code the Seed Infos 
	For seed-id length we will se S flag to 3 in current version */
	pstCurSeed = pstDomain->pstRemoteSeeds;
	while(pstCurSeed){
		ucCtrlMsgBuf[curBufIdx] = pstCurSeed->ucMinSequence;
		curBufIdx++;
		if (pstCurSeed->ucSeedIDLen == 16){
			ucCtrlMsgBuf[curBufIdx] |= MPL_SIDLEN_128;
		}
		else if(pstCurSeed->ucSeedIDLen == 8){
			ucCtrlMsgBuf[curBufIdx] |= MPL_SIDLEN_64;
		}
		else if(pstCurSeed->ucSeedIDLen == 2){
			ucCtrlMsgBuf[curBufIdx] |= MPL_SIDLEN_16;
		}
		
		bmlenIdx = curBufIdx;
		curBufIdx++;

		/* Encode the buffered message set */
		MplCtrlMsgEncodeBufferedMsgSet(pstCurSeed, ucCtrlMsgBuf + curBufIdx, 
									&ucBmlen);
		ucCtrlMsgBuf[bmlenIdx] = (ucBmlen << 2);
		curBufIdx += ucBmlen;
		pstCurSeed = pstCurSeed->pstNext;
	}

	return g_ExternalInterfaceApi.pfSendCtrlMsg(ucCtrlMsgBuf, curBufIdx, 
						MPL_ICMPv6_TYPE, MPL_ICMPv6_CODE);
}

