#ifndef __MPL_OSABSTRACTION_H__
#define __MPL_OSABSTRACTION_H__

typedef void *(*pfMplOSAbsMemcpy)
(
	void *pvdest, 
	MPL_SIZET destsize,
	void *pstSrc,
	MPL_SIZET numBytes
);

typedef void *(*pfMplOSAbsMemsset)
(
	void *pvdest, 
	MPL_SIZET destsize,
	MPL_INT iCharacter,
	MPL_SIZET numBytes
);


typedef struct _MplOsAbstractionsS
{
	pfMplOSAbsMemcpy pfMemcpy;
	pfMplOSAbsMemsset pfMemset;
}MplOsAbstractionsS;

MPL_INT MplRegisterSystemFunctions
(
	MplOsAbstractionsS *pstCallbacks
);

#endif /*__MPL_OSABSTRACTION_H__*/
