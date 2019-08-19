#ifndef __MPL_UTIL_H__
#define __MPL_UTIL_H__
#include "mpl-types.h"

#define MPL_UNIT_VALUE 1
#define MPL_BITS_INBYTE 8

int MplUtilGetFreeMemoryIndex
(
	MPL_UCHAR *pcMemory, 
	MPL_UINT itemMaxCnt	
);

void MplUtilFreeMemoryIndex
(
	MPL_UCHAR *pcMemory,
	int icbidx
);

#endif /*__MPL_UTIL_H__*/
