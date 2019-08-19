#ifndef __MPL_DATAMSG_H__
#define __MPL_DATAMSG_H__

#include "mpl-information.h"


#define MPL_SEEDID_LEN_0 0x00
#define MPL_SEEDID_LEN_16 0x40
#define MPL_SEEDID_LEN_64 0x80
#define MPL_SEEDID_LEN_128 0xC0

typedef struct _MplOptionS
{
	MPL_UCHAR optType;
	MPL_UCHAR optLen;
	MPL_UCHAR seedIdlen;
	MPL_UCHAR isLargestSeq;
	MPL_UCHAR ucSeqNo;
	MPL_UCHAR seedid[MPL_SEEDID_MAX];
}MplOptionS;

typedef struct _HbhOptionS
{
	MPL_UCHAR ucType;
	MPL_UCHAR ucLen;
}HbhOptionS;

MPL_INT MplDataMsgTransmit(MplBuffferedMessageS *pstBuffmsg, 
					MplDomainS *pstDomain);


#endif /*__MPL_DATAMSG_H__*/
