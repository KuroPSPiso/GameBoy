#ifndef MMU_H
#define MMU_H
#pragma once

#include "Defines.h"
#include "Cartidge.h"

class MMU
{
private:
	BOOL _IsInBios;
	uint8* _BIOS;
	uint8* _ROM;	//Cartridge ROM
	uint8* _VRAM;	//Video/GPU RAM (extends to OAM)
	uint8* _ERAM;	//Extrenal RAM
	uint8* _WRAM;	//Working RAM
	uint8* _OAM;	//Spriute Attribute Memory
	uint8* _IO;	//I/O interface memory
	uint8* _ZPRAM;	//Zero-Page (HIGH) RAM
	uint8 _IFlag;	//Interupt Flag
	Cartidge* _cartridge;

public:
	MMU(Cartidge* cartridge);
	~MMU();

private:
	void Set_Bios();

public:
	void Reset();

	uint8 MMU::Read8(uint16 address);
	uint16 MMU::Read16(uint16 address);
	void MMU::Write8(uint16 address, uint8 value);
	void MMU::Write16(uint16 address, uint16 value);
};
#endif // !MMU_H
