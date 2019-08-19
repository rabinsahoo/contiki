#ifndef __MPL_TRICKLE_H__
#define __MPL_TRICKLE_H__
#include "mpl-types.h"

#define MPL_TIMER_STATE_ACTIVE 0x01
#define MPL_TIMER_TYPE_CTRL 0x02
#define MPL_TIMER_TYPE_DATA 0x04

#ifdef MPL_IN_CONTIKI
#include "ctimer.h"
typedef struct ctimer MplTimerS;
#else
typedef void MplTimerS;
#endif


struct _MplTrickleTimerS;
typedef MPL_UINT (*pfTimeOutHandler)(struct _MplTrickleTimerS *pstTmr, void *pvUserData);

/* In the flags field bit 1 repesent if timer is active or not,
  bit 2 represent if its timer for control message 
  bit 3 represents if its timer for data message or not
  */
typedef struct _MplTrickleTimerS{
	pfTimeOutHandler pfTmOutHandler;
	void *pvUserData;
	MplTimerS TimerObj;
	MPL_UINT uiCurInterval;
	MPL_UINT uiRemainiCurInterval;
	MPL_UCHAR ucFlags;
	MPL_UCHAR ucCounter;
	MPL_UCHAR ucSendBuffmsg;
	MPL_UCHAR ucNumTrans;
}MplTrickleTimerS;

void MplTrickleInitialize(MplTrickleTimerS *pstTimer, pfTimeOutHandler pfCallback,
						MPL_BOOL isCrlMsgTimer);
void MplTrickleResetTimer(MplTrickleTimerS *pstTimer, void *pvUsrData);

MPL_VOID MplTrickleStartTimer(MplTrickleTimerS *pstTimer, 
					MPL_UINT uiDuration);


#define MPL_TRICKLE_ISACTIVE(pstTimer) \
	(pstTimer->ucFlags & MPL_TIMER_STATE_ACTIVE)


#endif /*__MPL_TRICKLE_H__*/

