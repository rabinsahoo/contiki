/* Defines basic data types used in MPL*/
#ifndef __MPL_TYPES_H__
#define __MPL_TYPES_H__
#include<inttypes.h>
#include<stddef.h>
#include<stdio.h>
#include <string.h>
#include <stdlib.h>

#define MPL_TRUE 1
#define MPL_FALSE 0

#define IN
#define OUT
#define IO

typedef int MPL_INT;
typedef short int MPL_SINT;
typedef unsigned int MPL_UINT;
typedef unsigned short int MPL_USINT;
typedef char MPL_CHAR;
typedef unsigned char MPL_UCHAR;
typedef int MPL_BOOL;
typedef size_t MPL_SIZET;
typedef void MPL_VOID;

typedef union _Ip6AddrS
{
  MPL_UCHAR u8[16];
  MPL_USINT u16[8];
  MPL_UINT u32[4];
}Ip6AddrS;

#endif /*__MPL_TYPES_H__*/

