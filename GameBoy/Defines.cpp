#include "Defines.h"

void Set_Bit(uint8 &value, uint8 bitNo, BOOL status)
{
	if (status == TRUE)
	{
		value |= (1 << bitNo);
	}
	else
	{
		value &= ~(1 << bitNo);
	}
}

void Set_Bit(uint16 &value, uint8 bitNo, BOOL status)
{
	if (status == TRUE)
	{
		value |= (1 << bitNo);
	}
	else
	{
		value &= ~(1 << bitNo);
	}
}

BOOL Get_Bit(uint8 value, uint8 bitNo)
{
	return (value >> bitNo) & 1;
}

BOOL Get_Bit(uint16 value, uint8 bitNo)
{
	return (value >> bitNo) & 1;
}

//Limited to positive numbers
uint32 POW(uint32 value, uint8 power)
{
	uint32 result = (power == 0) ? 1 : value;
	for (; power - 1 > 0; power--)
	{
		result *= value;
	}

	return result;
}
