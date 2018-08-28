#ifndef DEFINES_H
#define DEFINES_H

#define TRUE 0x01
#define FALSE 0x00

typedef int BOOL;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

void Set_Bit(uint8 &value, uint8 bitNo, BOOL status);
BOOL Get_Bit(uint8 value, uint8 bitNo);
BOOL Get_Bit(uint16 value, uint8 bitNo);
uint32 POW(uint32 value, uint8 multiplier);
#endif // !DEFINES_H
