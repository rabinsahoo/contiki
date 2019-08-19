#include "mpl-trickle.h"
#include "mpl-config.h"
#include "mpl-errno.h"
#include "mpl-interfaces.h"

#define MPL_RAND_MAX 65535

extern MplExternalInterfaceS g_ExternalInterfaceApi;

static MPL_USINT MplTrickleRandomRand(void)
{
	/* In gcc int rand() uses RAND_MAX and long random() uses 
	RANDOM_MAX=0x7FFFFFFF. RAND_MAX varies depending on the 
	architecture */
	
 	return (MPL_USINT)rand();
}

static void MplTrickleStartNewInterval(MplTrickleTimerS *pstTimer,
						void *pvUserData)
{
	MPL_UINT uiInterval; 
	MPL_UINT uiT;

	/* Here we will calculate the actual Interval I from the current interval 
	of the Trickle Timer*/
	uiInterval = 1 << pstTimer->uiCurInterval;

	/* We have got I, Now we need to find a value between [I/2, I). For this 
	we will generate a random value between I/2 and I*/
	pstTimer->uiRemainiCurInterval = uiInterval;
	uiT = uiInterval / 2 + (uiInterval / 2 * (uint32_t)MplTrickleRandomRand()) 
		/ MPL_RAND_MAX;
	pstTimer->uiRemainiCurInterval -= uiT;

	pstTimer->pvUserData = pvUserData;
	pstTimer->ucSendBuffmsg = 1;

	MplTrickleStartTimer(pstTimer, uiT);
}


static void MplTrickleTimerStop(MplTrickleTimerS *pstTrickle)
{
	if (pstTrickle->ucFlags & MPL_TIMER_TYPE_CTRL){
		pstTrickle->uiCurInterval = MPL_CONTROL_MESSAGE_IMIN +
			MPL_CONTROL_MESSAGE_INTVL_DBLING;
	}
	else if (pstTrickle->ucFlags & MPL_TIMER_TYPE_DATA){
		pstTrickle->uiCurInterval = MPL_DATA_MESSAGE_IMIN +
			MPL_DATA_MESSAGE_INTVL_DBLING;
	}

	/* Chnage the state to inavtive */
	pstTrickle->ucFlags &= ~(MPL_TIMER_STATE_ACTIVE);
}

MPL_VOID MplTrickleHandleTimeout(MPL_VOID *pvParam)
{
	MplTrickleTimerS *pstTimer = (MplTrickleTimerS *)pvParam;
	MPL_UINT uiRet;

  	if (pstTimer->ucSendBuffmsg){
		if ((pstTimer->ucCounter < MPL_DATA_MESSAGE_K)
			&& (pstTimer->ucNumTrans < MPL_DATA_MESSAGE_TIMER_EXPIRATIONS)){
			uiRet = pstTimer->pfTmOutHandler(pstTimer, pstTimer->pvUserData);
		}

		pstTimer->ucSendBuffmsg = 0;
		if (uiRet == MPL_RET_STOP_TRICKLE){
			MplTrickleTimerStop(pstTimer);
		}
		
		MplTrickleStartTimer(pstTimer, pstTimer->uiRemainiCurInterval);
	}
	else{
		/* IN creasse interval I by */
		if (pstTimer->uiCurInterval < MPL_DATA_MESSAGE_IMIN + 
				MPL_DATA_MESSAGE_K){
			pstTimer->uiCurInterval++;
		}

		MplTrickleStartNewInterval(pstTimer, pstTimer->pvUserData);
	}

	return;
}


//#ifdef MPL_IN_CONTIKI
MPL_VOID MplTrickleStartTimer(MplTrickleTimerS *pstTimer, 
				MPL_UINT uiDuration)
{

	g_ExternalInterfaceApi.pfSetTimer(&(pstTimer->TimerObj), uiDuration, 
						MplTrickleHandleTimeout, pstTimer);
#if 0
	clock_time_t ticks;

	ticks = (uiDuration * CLOCK_SECOND)/1000;
	ctimer_set(&(pstTimer->TimerObj), ticks, MplTrickleHandleTimeout, pstTimer);
#endif	
}
//#endif

void MplTrickleInitialize(MplTrickleTimerS *pstTimer, 
						pfTimeOutHandler pfCallback,
						MPL_BOOL isCrlMsgTimer)
{
	pstTimer->ucFlags = 0;
	pstTimer->pfTmOutHandler = pfCallback;
	pstTimer->ucCounter = 0;
	if (isCrlMsgTimer){
		pstTimer->uiCurInterval = MPL_CONTROL_MESSAGE_IMIN +
			MPL_CONTROL_MESSAGE_INTVL_DBLING;
		pstTimer->ucFlags |= MPL_TIMER_TYPE_CTRL;
	}
	else{
		pstTimer->uiCurInterval = MPL_DATA_MESSAGE_IMIN +
			MPL_DATA_MESSAGE_INTVL_DBLING;
		pstTimer->uiRemainiCurInterval = 0;
		pstTimer->ucFlags |= MPL_TIMER_TYPE_DATA;
	}
}
						
void MplTrickleResetTimer(MplTrickleTimerS *pstTimer, void *pvUsrData)
{
	if (pstTimer->uiCurInterval > MPL_DATA_MESSAGE_IMIN){
		pstTimer->uiCurInterval = MPL_DATA_MESSAGE_IMIN;
		pstTimer->ucCounter = 0;
		pstTimer->uiCurInterval = 0;
		MplTrickleStartNewInterval(pstTimer, pvUsrData);
	}
}

						
