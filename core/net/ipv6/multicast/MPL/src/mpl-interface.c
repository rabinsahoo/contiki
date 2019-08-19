#include "mpl-interfaces.h"
#include "mpl-ctrlmsg.h"
#include "mpl-information.h"

MplDomainS g_MplDomainArray[MPL_NUM_MPL_DOMAINS];

MPL_UCHAR g_UsedArr[MPL_NUM_MPL_DOMAINS];

MplOsAbstractionsS g_SysCallbacks = {NULL};

MplExternalInterfaceS g_ExternalInterfaceApi;

#define IS_SYS_CALLBACKS_VALID()\
	(g_SysCallbacks.pfMemcpy && g_SysCallbacks.pfMemset)

MPL_INT MplRegisterSystemFunctions
(
	MplOsAbstractionsS *pstCallbacks
)
{
	if ((NULL == pstCallbacks) || (!pstCallbacks->pfMemcpy)
		|| (!pstCallbacks->pfMemset)){
		return MPL_RET_FAILURE;
	}

	g_SysCallbacks.pfMemcpy = pstCallbacks->pfMemcpy;
	g_SysCallbacks.pfMemset = pstCallbacks->pfMemset;

	return MPL_RET_SUCCESS;
}

MPL_INT MplApiComponentInit
(
	IN MplExternalInterfaceS *pstInterfaceAPIs
)
{
	if (!IS_SYS_CALLBACKS_VALID())
	{
		return MPL_RET_FAILURE;
	}

	/* Validate the parameters */
	if ((NULL == pstInterfaceAPIs->pfSendCtrlMsg) ||
		(NULL == pstInterfaceAPIs->pfSenddataMsg) ||
		(NULL == pstInterfaceAPIs->pfSetTimer) ||
		(NULL == pstInterfaceAPIs->pfStopTimer))
	{
		printf("Invalid parameter\n");
		return MPL_RET_FAILURE;
	}

	g_ExternalInterfaceApi.pfSendCtrlMsg = pstInterfaceAPIs->pfSendCtrlMsg;
	g_ExternalInterfaceApi.pfSenddataMsg = pstInterfaceAPIs->pfSenddataMsg;
	g_ExternalInterfaceApi.pfSetTimer = pstInterfaceAPIs->pfSetTimer;
	g_ExternalInterfaceApi.pfStopTimer = pstInterfaceAPIs->pfStopTimer;
	
	MPL_MEMSET(&g_MplDomainArray, sizeof(g_MplDomainArray),
	 0, sizeof(g_MplDomainArray));
	
	MPL_MEMSET(&g_UsedArr, sizeof(g_UsedArr), 0, sizeof(g_UsedArr));

	return MPL_RET_SUCCESS;
}


static MplDomainS* MplGetFreeDomain(void)
{
	MPL_INT idx = 0;

	for (; idx <MPL_NUM_MPL_DOMAINS; idx++)
	{
		if (!g_UsedArr[idx])
		{
			g_UsedArr[idx] = MPL_TRUE;
			return&g_MplDomainArray[ idx];
		}
	}

	return NULL;
}


static MplDomainS *MplFindDomainByDomainId
(
 	Ip6AddrS *domainAddr
)
{
	MPL_INT idx;

	for (idx = 0; idx < MPL_NUM_MPL_DOMAINS; idx++)
	{
		if (g_UsedArr[idx] && 
			!memcmp(&(g_MplDomainArray[idx].stDomainAddr) ,
			domainAddr, sizeof(Ip6AddrS)))
		{			
			return &(g_MplDomainArray[idx]);
		}
	}

	return NULL;
}

 MplDomainS *MplFindDomainByLLDomainId
(
 	Ip6AddrS *domainAddr
)
{
	MPL_INT idx;
	Ip6AddrS stLinkLocal = {{0}};

	for (idx = 0; idx < MPL_NUM_MPL_DOMAINS; idx++)
	{
		if (g_UsedArr[idx] )
		{
			stLinkLocal = g_MplDomainArray[idx].stDomainAddr;
			stLinkLocal.u16[0] = 0xff02;
			if (!memcmp(domainAddr, &stLinkLocal, sizeof(Ip6AddrS))){				
				return &(g_MplDomainArray[idx]);
			}
		}
	}

	return NULL;
}

MPL_INT MplApiAddDomain
(
	IN MPL_UINT interfaceIdx, 
	IN Ip6AddrS *DomainAddr,
	IN Ip6AddrS *IfaceUcastAddrLst,
	IN MPL_UCHAR numAddr
)
{
	MplDomainS *pstMplDomain;
	MplInterfaceSetS *pstInterface;
	int idx;

	/* Check if a domain already exists */
	pstMplDomain = MplFindDomainByDomainId(DomainAddr);
	if (NULL == pstMplDomain)
	{
		printf("MPL Domain already exists\n");
		return MPL_RET_FAILURE;
	}

	pstMplDomain = MplGetFreeDomain();
	if (NULL == pstMplDomain)
	{
		return MPL_RET_FAILURE;
	}

	MPL_MEMSET(pstMplDomain, sizeof(MplDomainS), 0, sizeof(MplDomainS));

	MPL_MEMCPY(&(pstMplDomain->stDomainAddr),
		sizeof(Ip6AddrS), (void *)DomainAddr, sizeof(Ip6AddrS));

	/* This is the first interface so we can directly add to 1st interface*/
	pstInterface = &(pstMplDomain->stIfaceSet[0]);
	pstInterface->interfaceIdx = interfaceIdx;

	for ( idx = 0; idx <numAddr ; idx++)
	{
		MPL_MEMCPY(&(pstInterface->addrrset[idx]), sizeof(Ip6AddrS), 
			&(IfaceUcastAddrLst[idx]), sizeof(Ip6AddrS));
	}

	MplTrickleInitialize(&(pstMplDomain->stCtrlMsgTmr), 
				MplCtrlMsgTimeOutHandler, MPL_TRUE);

	MplInfoSetStartMplDomain(pstMplDomain);

	pstInterface->isUsed = 1;

	return MPL_RET_SUCCESS;
}

MPL_INT MplApiRemoveDomain
(	IN Ip6AddrS *DomainAddr
)
{
	MplDomainS *pstDm;
	MPL_INT idx;
	
	pstDm = MplFindDomainByDomainId(DomainAddr);
	if (NULL == pstDm)
	{
		printf("Didn't find the MPL domain\n");
		return MPL_RET_FAILURE;
	}

	idx = pstDm - (&g_MplDomainArray[0]);
	g_UsedArr[idx] = MPL_FALSE;

	return MPL_RET_SUCCESS;
}

MPL_INT MplApiAddLocalMplSeed
(
	IN Ip6AddrS *DomainAddr,
	IN MPL_UCHAR *mplSeedId,
	IN MPL_UCHAR ucSeedLen
)
{
	MplDomainS *mpldomain;

	/* Validate the parameters */
	if (NULL == DomainAddr || NULL == mplSeedId || ((0 == ucSeedLen) || 
			(ucSeedLen > MPL_SEEDID_MAX)) )
	{
		printf("Invalid parameters\n");
		return MPL_RET_FAILURE;
	}

	/* Find the MPL domain */
	mpldomain = MplFindDomainByDomainId(DomainAddr);
	if (NULL == mpldomain)
	{
		printf("Didn't find the MPL domain\n");
		return MPL_RET_FAILURE;
	}

	MPL_MEMCPY(mpldomain->stSeedInfo.ucSeedID, MPL_SEEDID_MAX, mplSeedId, 
			ucSeedLen);
	mpldomain->stSeedInfo.ucSeedIDLen = ucSeedLen;
	mpldomain->stSeedInfo.uiSendSeqNum = rand();

	return MPL_RET_SUCCESS;
}

MplDomainS *MplGetMplDomain
(
	Ip6AddrS *pstDomainAddr,
	MPL_UINT uiIfaceIndex
)
{
	MplDomainS *pstdmn;
	MplInterfaceSetS *ifaceset;
	MPL_INT idx;

	pstdmn = MplFindDomainByDomainId(pstDomainAddr);
	if (NULL == pstdmn)
	{
		printf("Didn't find the domain\n");
		return NULL;
	}

	/* Verify the interface */
	for (idx = 0; idx <MPL_INTERFACESET_SIZE; idx++)
	{
		ifaceset = &(pstdmn->stIfaceSet[idx]);
		if (ifaceset->interfaceIdx == uiIfaceIndex)
		{
			return pstdmn;
		}
	}

	return NULL;
}


