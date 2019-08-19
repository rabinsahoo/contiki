/* It defines the possible error codes used in the MPL */
#ifndef __MPL_ERRNO_H__
#define __MPL_ERRNO_H__

typedef enum _MplRetCodeE
{
 	MPL_RET_FAILURE = -1,
 	MPL_RET_SUCCESS,
 	MPL_RET_DROP_MCAST_PACKET, 
 	MPL_RET_PROC_MCAST_PACKET,
 	MPL_RET_DETECTED_INCONSISTENCY,
 	MPL_RET_CONTINUE_TRICKLE,
 	MPL_RET_STOP_TRICKLE
}MplRetCode;

#endif /*__MPL_ERRNO_H__*/

