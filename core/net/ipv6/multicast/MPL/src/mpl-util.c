#include "mpl-util.h"

int MplUtilGetFreeMemoryIndex
(
	MPL_UCHAR *pcMemory, 
	MPL_UINT itemMaxCnt
	
)
{
    int ibyte = 0;
    int ibit = 0;
    int numscan = 0;
    unsigned char locval;

    while (numscan < itemMaxCnt){
        locval = pcMemory[ibyte];

        /* Find which bit is not set */
        for (ibit = 7; ibit >= 0  && numscan < itemMaxCnt;  ibit--, numscan++){
            if (!(locval & (MPL_UNIT_VALUE << ibit))){
                pcMemory[ibyte] |= (MPL_UNIT_VALUE << ibit);
                printf("Empty Location index [%u]\n",(ibyte * 8) + (8 - (ibit+1)));
                return ((ibyte * MPL_BITS_INBYTE) + (MPL_BITS_INBYTE - (ibit+1)));
            }
        }

        ibyte++;
    }

    return -1;
}

void MplUtilFreeMemoryIndex
(
	MPL_UCHAR *pcMemory,
	int icbidx
)
{
    int byteloc;
    int bitpos;

    byteloc = (icbidx/MPL_BITS_INBYTE);
    bitpos = ((byteloc * MPL_BITS_INBYTE) - icbidx);
    bitpos = (MPL_BITS_INBYTE - (bitpos + 1));

    printf("Byte Loc [%d] Bit Pos[%d]\n",byteloc, bitpos);
    printf("Before reset [%0x]\n",pcMemory[byteloc]);
	
    pcMemory[byteloc] &= (~(1 << bitpos));

    printf("After reset [%0x]\n",pcMemory[byteloc]);

	return;
}


/* BST To maiantain all buffered mesages */
