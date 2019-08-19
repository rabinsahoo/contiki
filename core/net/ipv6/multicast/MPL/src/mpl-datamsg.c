#include "mpl-information.h"
#include "mpl-datamsg.h"
#include "mpl-ctrlmsg.h"

#define MPL_IPv6_HBH_HEADER 0
#define MPL_IPv6_HDR_SIZE 40u
#define MPL_HBH_HEADER_SIZE 2u
#define MPL_OPTION_FIXED_LEN 6u

#define EXT_HDR_OPT_PAD1  0
#define EXT_HDR_OPT_PADN  1
#define EXT_HDR_OPT_RPL 0x63
#define EXT_HDR_OPT_RPL_NEW 0x23
#define EXT_HDR_OPT_MPL 0x6D
#define MPL_FLAG_V 0x10
#define MPL_FLAG_M 0x20

#define MPL_HBH_OPTION_TYPE(hbhhdrbuff, exthdroffset) \
	((HbhOptionS *)(pcMplOptBuf + usoffset))->ucType

#define MPL_HBH_OPTION ((HbhOptionS *)(pcMplOptBuf + usoffset))

#define MPL_UPDATE_VAR(ullen) \
	usoffset += ullen; \
	usRemainLen -= ullen; \
	hbhexthdrlen -= ullen;

extern MplExternalInterfaceS g_ExternalInterfaceApi;

static MPL_INT MplProcessMplOption
(
	MPL_UCHAR *pcMplOptBuf,
	MplOptionS *mplOption
)
{
	MPL_UCHAR flags;
	
	mplOption->optType = pcMplOptBuf[0];
	mplOption->optLen = pcMplOptBuf[1];
	flags = pcMplOptBuf[ 2];

	/* Check if V flag is set to 1 in that case drop the packet */
	if (flags & MPL_FLAG_V)
	{
		printf("Flag V is set to 1 we will drop the packet\n");
		return MPL_RET_DROP_MCAST_PACKET;
	}
				
	if (flags & MPL_SEEDID_LEN_16)
	{
		mplOption->seedIdlen = 2;
	}
	else if (flags & MPL_SEEDID_LEN_64)
	{
		mplOption->seedIdlen = 8;
	}
	else if (flags & MPL_SEEDID_LEN_128)
	{
		mplOption->seedIdlen = 16;
	}
	else
	{
		mplOption->seedIdlen = 0;
	}

	/* Check for M flag */
	if (flags & MPL_FLAG_M)
	{
		mplOption->isLargestSeq = 1;
	}

	/*Get the data message sequence number*/
	mplOption->ucSeqNo = pcMplOptBuf[3];

	/* Copy the seed-id if length is valid */
	if (mplOption->seedIdlen)
	{
		MPL_MEMCPY(mplOption->seedid, MPL_SEEDID_MAX, pcMplOptBuf + 4, 
			mplOption->seedIdlen);
	}

	return MPL_RET_SUCCESS;
}

MPL_INT MplProccessHbhForMplOption
(
	MPL_UCHAR *ucIpv6Packet,
	MPL_USINT usPacketLen,
	MplOptionS *mplOption
)
{
	MplIPv6HdrS *pstIp6hdr;
	MPL_UCHAR *pcMplOptBuf;
	MPL_USINT usRemainLen;
	MPL_USINT hbhexthdrlen;
	MPL_USINT usoffset;
	
	/* Check if the next header of IPv6 header is HBH header or not*/
	pstIp6hdr = (MplIPv6HdrS *)ucIpv6Packet;

	if (pstIp6hdr->nextHdr != MPL_IPv6_HBH_HEADER)
	{
		printf("Didn't find the HBH header found\n");
		return MPL_RET_DROP_MCAST_PACKET;
	}

	usRemainLen = usPacketLen - (MPL_USINT)(MPL_IPv6_HDR_SIZE);

	/* Validate the length of the IPv6 packet against the HBH headersize*/
	if (usRemainLen < MPL_HBH_HEADER_SIZE)
	{
		printf("Corrupt packet\n");
		return MPL_RET_DROP_MCAST_PACKET;			
	}
	
	pcMplOptBuf = ucIpv6Packet + MPL_IPv6_HDR_SIZE;
	hbhexthdrlen = (MPL_USINT)(pcMplOptBuf[1] ) * 8 + 8;

	usRemainLen -= MPL_HBH_HEADER_SIZE;
	if (usRemainLen < hbhexthdrlen)
	{
		printf("Corrupt IP packet\n");
		return MPL_RET_DROP_MCAST_PACKET;
	}

	usoffset = MPL_HBH_HEADER_SIZE;

	while (hbhexthdrlen)
	{
		if (usRemainLen < 2){
			printf("Corrupt packet\n");
			return MPL_RET_DROP_MCAST_PACKET;
		}
		
		switch(MPL_HBH_OPTION->ucType)
		{ 
			case EXT_HDR_OPT_PAD1:
				usoffset += 1;
				usRemainLen -= 1;
				break;
			case EXT_HDR_OPT_PADN:
				if (usRemainLen < (MPL_HBH_OPTION->ucLen + 2))
				{
					printf("Corrupt packet\n");
					return MPL_RET_DROP_MCAST_PACKET;
				}
				
				MPL_UPDATE_VAR((MPL_HBH_OPTION->ucLen + 2));
				break;
			case EXT_HDR_OPT_RPL:
			case EXT_HDR_OPT_RPL_NEW:
				if (usRemainLen < (MPL_HBH_OPTION->ucLen + 2))
				{
					printf("Corrupt packet\n");
					return MPL_RET_DROP_MCAST_PACKET;
				}
				
				MPL_UPDATE_VAR((MPL_HBH_OPTION->ucLen + 2));
				break;
			case EXT_HDR_OPT_MPL:
				if (usRemainLen < (MPL_HBH_OPTION->ucLen + 2))
				{
					printf("Corrupt packet\n");
					return MPL_RET_DROP_MCAST_PACKET;
				}
				
				return MplProcessMplOption(pcMplOptBuf + hbhexthdrlen, mplOption);			
			default:
				if (usRemainLen < (MPL_HBH_OPTION->ucLen + 2))
				{
					printf("Corrupt packet\n");
					return MPL_RET_DROP_MCAST_PACKET;
				}
				
				MPL_UPDATE_VAR((MPL_HBH_OPTION->ucLen + 2));
		}			
	}

	return MPL_RET_DROP_MCAST_PACKET;
}

static MPL_INT MplnfoSetAddOrUpdateMplSeed
(
	MplDomainS *pstMplDomain,
	MplOptionS *pstMplOpt,
	MPL_UCHAR *ucMcastMsg,
	MPL_USINT usMsgLen,
	MPL_UINT uiIfIndex
)
{
	MplSeedSetS *pstSeed = NULL;
	MplBuffferedMessageS *pstNewBuffSetEntry = NULL;
	
	/* Check in the Seed exist with the given seed id*/
	pstSeed = MplInfoSetGetSeedBySeedID(pstMplDomain, pstMplOpt->seedid, 
					pstMplOpt->seedIdlen);
	if (NULL == pstSeed)
	{
		/*Add a new seed to the seed set */
		pstSeed = MplInfoSetAllocSeedEntry();
		if (NULL == pstSeed){
			printf("Didn't find  free seed set entry to add the seed info\n");
			return MPL_RET_FAILURE;
		}

		MPL_MEMSET(pstSeed, sizeof(MplSeedSetS), 0, sizeof(MplSeedSetS));

		/* Largets sequence number flag need to be handled */
		pstSeed->ucMinSequence = pstMplOpt->ucSeqNo;
		pstSeed->ucSeedIDLen = pstMplOpt->seedIdlen;
		MPL_MEMCPY(pstSeed->ucSeedID, MPL_SEEDID_MAX, pstMplOpt->seedid, 
			pstMplOpt->seedIdlen);
		pstSeed->uiRemainLifeTime = MPL_SEED_SET_ENTRY_LIFETIME;
		
		/* Add the seed entry to seed set list */
		MplInfoSetAddSeed2SeedSetList(pstMplDomain, pstSeed);
	}
	else
	{
		/* Check if the received Data messageis old or duplicate one */
		if (pstSeed->ucMinSequence >= pstMplOpt->ucSeqNo)
		{
			printf("Received old packet min seq[%u] recvd seq[%d]\n", 
				pstSeed->ucMinSequence, pstMplOpt->ucSeqNo);
			return MPL_RET_FAILURE;
		}

		/* Check if its the duplicate on */
		if (MplInoSetIsExistBufferedMsgSetEntry(pstSeed, pstMplOpt->ucSeqNo))
		{
			/* Here Set the trickle redundancy for this buffered message 
			if its trickle timer is running */
			printf("Duplicate data message\n");
			return MPL_RET_PROC_MCAST_PACKET;
		}

		/* We are accepting the received MPL Data message so reset the seed set 
		entry lifetime back to SEED_SET_ENTRY_LIFETIME*/
		pstSeed->uiRemainLifeTime = MPL_SEED_SET_ENTRY_LIFETIME;
	}

	/* Add to the buffered message set */
	pstNewBuffSetEntry = MplInfoSetAllocNewBufferSetEntry();
	if (NULL == pstNewBuffSetEntry)
	{
		/* There is no free space so remove some of the entries from the 
		buffered message set */
		MplInfoSetReclaimMemory(pstMplDomain, pstSeed);
		pstNewBuffSetEntry = MplInfoSetAllocNewBufferSetEntry();
		if (NULL == pstNewBuffSetEntry)
		{
		 	printf("Failed to get free space\n");
			return MPL_RET_PROC_MCAST_PACKET;
		}
	}

	pstNewBuffSetEntry->MsgLe = usMsgLen;
	pstNewBuffSetEntry->ucIfIndex = (MPL_UCHAR)uiIfIndex;
	pstNewBuffSetEntry->ucSequence = pstMplOpt->ucSeqNo;
	MPL_MEMCPY(pstNewBuffSetEntry->buff, MPL_IPV6_MTU, ucMcastMsg, usMsgLen);

	if (!pstSeed->pstBufMsgSet){
		pstSeed->pstBufMsgSet = pstNewBuffSetEntry;
		pstNewBuffSetEntry->pstNext = NULL;
	}
	else{
		pstNewBuffSetEntry->pstNext = pstSeed->pstBufMsgSet;
		pstSeed->pstBufMsgSet = pstNewBuffSetEntry;
	}
	
	/* If PROACTIVE_FORWARDING is enables initialize and start trickle timer 
	for the received data message*/
	MplTrickleInitialize(&(pstNewBuffSetEntry->stTrickleTimer), MplInfoSetDataMsgTimeOutHandler, 
					MPL_FALSE);

#if MPL_PROACTIVE_FORWARDING
	MplInfoSetStartOrResetBuferMsgTimer(pstNewBuffSetEntry);
#endif

	return MPL_RET_SUCCESS;
}


MPL_INT MplProcessMcastMessage
(
	IN MPL_UCHAR *ucIpv6Packet,
	IN MPL_USINT usPacketLen,
	IN MPL_UINT uiIfaceIdx
)
{
	MplDomainS *pstMpldomain;
	MplOptionS stMplOption;
	MPL_INT ret;
	

	/* Check if MPL domain is there*/	
	pstMpldomain = MplGetMplDomain(&MPL_PKT_IPV6_DSTADDR(ucIpv6Packet), uiIfaceIdx);
	if (NULL == pstMpldomain)
	{
		printf("Node is not part of this MPL domain\n ");
		return MPL_RET_DROP_MCAST_PACKET;
	}

	ret = MplProccessHbhForMplOption(ucIpv6Packet, usPacketLen,&stMplOption);
	if (ret != MPL_RET_SUCCESS)
	{
		printf("Failed to process MPL option\n");
		return ret;
	}

	/* If seed ID length is zero then source address is seed id. Copy the seed 
	id*/
	if (!stMplOption.seedIdlen)
	{
		MPL_MEMCPY(stMplOption.seedid, MPL_SEEDID_MAX, 
			&MPL_PKT_IPV6_SRCADDR(ucIpv6Packet), sizeof(Ip6AddrS));
		stMplOption.seedIdlen = sizeof(Ip6AddrS);
	}

	/* Add or update the seed info and add message to buffered set */
	ret = MplnfoSetAddOrUpdateMplSeed(pstMpldomain, &stMplOption, ucIpv6Packet, 
							usPacketLen, uiIfaceIdx);
	if (ret){
	}

	return ret;
}

MPL_INT MplDataMsgTransmit(MplBuffferedMessageS *pstBuffmsg, 
					MplDomainS *pstDomain)
{
	return g_ExternalInterfaceApi.pfSenddataMsg(pstBuffmsg->buff, 
							pstBuffmsg->MsgLe);
}

