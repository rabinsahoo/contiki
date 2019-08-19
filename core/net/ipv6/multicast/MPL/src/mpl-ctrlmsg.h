#ifndef __MPL_CTRLMSG_H__
#define __MPL_CTRLMSG_H__

#include "mpl-information.h"
//#include "mpl-trickle.h"

/* IPV6 header */
typedef struct _MplIPv6HdrS{
	MPL_UINT vtf;   /*Version, traffic class and flow control */
	MPL_USINT payloadLen;
	MPL_UCHAR nextHdr;
	MPL_UCHAR hoplimit;
	Ip6AddrS srcaddr;
	Ip6AddrS dstaddr;
}MplIPv6HdrS;

typedef struct _MplIcmpv6HdrS{
	MPL_UCHAR type;
	MPL_UCHAR code;
	MPL_USINT checksum; 
}MplIcmpv6HdrS;

#define MPL_PKT_IPV6_SRCADDR(packetbuff) \
	((MplIPv6HdrS *)packetbuff)->srcaddr

#define MPL_PKT_IPV6_DSTADDR(packetbuff) \
	((MplIPv6HdrS *)packetbuff)->dstaddr

#define MPL_CTRL_MSG_MAX_SIZE 1280

MPL_INT MplCtrlMsgSend
(
	MplDomainS *pstDomain
);


#endif /*__MPL_CTRLMSG_H__*/

