/* It defines the data structures to represnt the MPL information base as 
   per the RFC 7731
*/
#ifndef __MPL_INFORMATION_H__
#define __MPL_INFORMATION_H__

#include "mpl-config.h"
#include "mpl-types.h"
#include "mpl-osabstraction.h"
#include "mpl-trickle.h"
#include "mpl-interfaces.h"

#define MPL_NUM_ADDRESS 1
#define MPL_INTERFACESET_SIZE 1
#define MPL_SEEDID_MAX 16

extern MplOsAbstractionsS g_SysCallbacks;

typedef MplTimerS MplPeriodicTimer;

#define MPL_MEMCPY(dest, dstsize, src, bytes2copy) \
	g_SysCallbacks.pfMemcpy(dest, dstsize, src, bytes2copy)

#define MPL_MEMSET(dest, dstsize, byte, bytes2copy) \
	g_SysCallbacks.pfMemset(dest, dstsize, byte, bytes2copy)

/*Define the MPL Interface*/
typedef struct _MplInterfaceSetS
{
	MPL_UINT interfaceIdx;
	Ip6AddrS addrrset[MPL_NUM_ADDRESS];
	MPL_UCHAR isUsed;
}MplInterfaceSetS;

typedef struct _MplBuffferedMessageS
{
	struct _MplBuffferedMessageS *pstNext;
	MplTrickleTimerS stTrickleTimer;
 	MPL_UCHAR buff[MPL_IPV6_MTU];
	MPL_USINT MsgLe;
	MPL_UCHAR ucSequence;	
	MPL_UCHAR ucIfIndex;
}MplBuffferedMessageS;

typedef struct _MplSeedSetS
{
	struct _MplSeedSetS *pstNext;
	MPL_UCHAR ucSeedID[MPL_SEEDID_MAX];
	MPL_UCHAR ucSeedIDLen;
	MPL_UCHAR ucMinSequence;
	MPL_UINT uiRemainLifeTime;
	MplBuffferedMessageS *pstBufMsgSet;
}MplSeedSetS;

typedef struct _MplSeedS
{
	MPL_UCHAR ucSeedID[MPL_SEEDID_MAX];
	MPL_UCHAR ucSeedIDLen;
	MPL_UINT uiSendSeqNum;
}MplSeedS;

/*Define MPL Domain */
typedef struct _MplDomainS
{
	Ip6AddrS stDomainAddr;
	MplTrickleTimerS stCtrlMsgTmr;
	MplPeriodicTimer stPeriodicTmr;
	MplInterfaceSetS stIfaceSet[MPL_INTERFACESET_SIZE];
	MplSeedS stSeedInfo;
	MplSeedSetS *pstRemoteSeeds;
	MPL_USINT usRSeedsCnt;
}MplDomainS;


MplDomainS *MplGetMplDomain
(
	Ip6AddrS *pstDomainAddr,
	MPL_UINT interfaceidx
);

#if 0
MPL_INT MplnfoSetAddOrUpdateMplSeed
(
	MplDomainS *pstMplDomain,
	MplOptionS *pstMplOpt,
	MPL_UCHAR *ucMcastMsg,
	MPL_USINT usMsgLen,
	MPL_UINT uiIfIndex
);
#endif

 MplSeedSetS *MplInfoSetAllocSeedEntry(void);

 MPL_BOOL MplInoSetIsExistBufferedMsgSetEntry
 (
	 MplSeedSetS *pstSeedSet,
	 MPL_UCHAR ucSeqNo
 );

 MplBuffferedMessageS *MplInfoSetAllocNewBufferSetEntry(void);

 MplDomainS *MplFindDomainByLLDomainId
(
 	Ip6AddrS *domainAddr
);

MplSeedSetS *MplInfoSetGetSeedBySeedID
(
	MplDomainS *pstMplDomain,
	MPL_UCHAR *ucSeedId,
	MPL_UCHAR  ucSeedIdLen
);

MPL_VOID MplInfoSetSetSeedScanBit
(
	MplSeedSetS *pstSeed
);

MPL_VOID MplInfoSetStartOrResetCtrlMsgTimer
(
	MplDomainS *pstDomain
);

MPL_VOID MplInfoSetStartOrResetBuferMsgTimer
(
	MplBuffferedMessageS *pstBuffMsg
);

MPL_UINT MplInfoSetDataMsgTimeOutHandler(MplTrickleTimerS *pstTmr, 
									void *pvUserData);


MPL_UINT MplCtrlMsgTimeOutHandler(MplTrickleTimerS *pstTmr, void *pvUserData);

MPL_VOID MplInfoSetFindMissingSeedInPeer(MplDomainS *pstDomain);

 void MplInfoSetAddSeed2SeedSetList
(
	MplDomainS *pstMplDomain,
	MplSeedSetS *pstSeedEntry
);

 void MplInfoSetReclaimMemory
(
	MplDomainS *pstMplDomain,
	MplSeedSetS *pstSeedSetEntry
);

MPL_VOID MplInfoSetStartMplDomain(MplDomainS *pstDomain);

#endif /*__MPL_INFORMATION_H__*/

