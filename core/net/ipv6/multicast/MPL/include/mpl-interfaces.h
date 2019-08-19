#ifndef __MPL_INTERFACE_H__
#define __MPL_INTERFACE_H__

#include "mpl-config.h"
#include "mpl-types.h"
#include "mpl-errno.h"

/* Interface to send MPL control message */
typedef MPL_INT (*pfMplIcmpv6Send)(MPL_UCHAR *pucMplmsgBuf, Ip6AddrS *pstDstAddr,
								MPL_USINT usMsgLen, MPL_UCHAR ucIcmpType, 
								MPL_UCHAR ucIcmp6Code);

typedef MPL_INT(*pfMplSendIpv6McastMsg)(MPL_UCHAR *pucDataMsg, 
										MPL_USINT usMsgLen);

typedef MPL_VOID (*pfTimeoutHdl)(MPL_VOID *pvData);

typedef MPL_INT (*pfMplStartTimer)(MPL_VOID *pvTimer, MPL_UINT uiDuration, 
							pfTimeoutHdl pfHandler, MPL_VOID *pvUserData);
typedef MPL_INT (*pfMplStopTimer)(MPL_VOID *pvTimer);

typedef struct _MplExternalInterfaceS{
	pfMplIcmpv6Send pfSendCtrlMsg;
	pfMplSendIpv6McastMsg pfSenddataMsg;
	pfMplStartTimer pfSetTimer;
	pfMplStopTimer pfStopTimer;	
}MplExternalInterfaceS;

MPL_INT MplApiComponentInit
(
	IN MplExternalInterfaceS *pstInterfaceAPIs
);

MPL_INT MplApiAddDomain
(
	IN MPL_UINT interfaceIdx, 
	IN Ip6AddrS *DomainAddr,
	IN Ip6AddrS *IfaceUcastAddrLst,
	IN MPL_UCHAR numAddr
);

MPL_INT MplApiRemoveDomain
(
	IN Ip6AddrS *DomainAddr
);

MPL_INT MplApiAddLocalMplSeed
(
	IN Ip6AddrS *DomainAddr,
	IN MPL_UCHAR *mplSeedId,
	IN MPL_UCHAR ucSeedLen	
);

MPL_INT MplProcessMcastMessage
(
	IN MPL_UCHAR *ucIpv6Packet,
	IN MPL_USINT usPacketLen,
	IN MPL_UINT uiIfaceIdx
);

MPL_VOID MplCtrlMsgProcessInput
(
	IN MPL_UCHAR *pcIcmpv6MsgBuf,
	IN MPL_USINT usMsgLen,
	IN Ip6AddrS *pstSrcAddr,
	IN Ip6AddrS *pstDstAddr
);

#endif /*__MPL_INTERFACE_H__*/

