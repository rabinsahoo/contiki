#include "mpl-interfaces.h"
#include "ctimer.h"
#include "uip.h"
#include "uip-icmp6.h"


#define UIP_ICMP_PAYLOAD ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_IP_BUF (uip_buf + UIP_LLH_LEN)


/* We need destination address  and the outgoing interface details */
 MPL_INT roll_mpl_icmpv6_send(MPL_UCHAR *pucMplmsgBuf, Ip6AddrS *pstDstAddr,
						 MPL_USINT usMsgLen, MPL_UCHAR ucIcmpType,
						 MPL_UCHAR ucIcmp6Code)
{
	uip_ip6addr_t stDstAddr;

	stDstAddr = *(uip_ip6addr_t *)pstDstAddr;
	memcpy(UIP_ICMP_PAYLOAD, usMsgLen, pucMplmsgBuf, ucIcmpType, ucIcmp6Code);
	uip_icmp6_send(&stDstAddr, ucIcmpType, ucIcmp6Code, usMsgLen);
	return 0;
}

MPL_INT roll_mpl_ipv6_send(MPL_UCHAR *pucDataMsg, 
								MPL_USINT usMsgLen)
{
	/* Copy the packet to the UIP buffre*/
	memcpy(UIP_IP_BUF, pucDataMsg, usMsgLen);
	uip_len = usMsgLen;
	tcpip_ipv6_output(void);

	return 0;
}

 MPL_INT roll_mpl_starttimer(MPL_VOID *pvTimer, MPL_UINT uiDuration, 
							pfTimeoutHdl pfHandler, MPL_VOID *pvUserData)
{	
	ctimer_set((struct ctimer *)pvTimer, uiDuration, pfHandler, pvUserData);
}
							
MPL_INT roll_mpl_stoptimer(MPL_VOID *pvTimer)
{
	ctimer_stop((struct ctimer * )pvTimer);
}

MplExternalInterfaceS g_FuncInterface2Mpl =
{
	.pfSendCtrlMsg = roll_mpl_icmpv6_send,
	.pfSenddataMsg = roll_mpl_ipv6_send,
	.pfSetTimer = roll_mpl_starttimer,
	.pfStopTimer = roll_mpl_stoptimer,
};

MplOsAbstractionsS g_OsAbstraction =
{
	.pfMemcpy = memcpy,
	.pfMemset = memset,
};

void roll_mpl_init(void)
{	
	MplRegisterSystemFunctions(&g_OsAbstraction);
	MplApiComponentInit(&g_FuncInterface2Mpl);
}

/* To add MPL HBH option to the data messages */
void roll_mpl_out(void)
{
	return;
}

/* Function to receive the incoming data mesages.*/
unsigned char roll_mpl_input()
{
	unsigned char ucRet;

	
	return ucRet;
}

const uip_mcast6_driver roll_mpl_driver = 
{
	"ROLLMPL",
	roll_mpl_init,
	roll_mpl_out,
	roll_mpl_input
};

