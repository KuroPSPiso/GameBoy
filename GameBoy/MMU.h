#ifndef MMU_H
#define MMU_H
#pragma once

#include "Defines.h"
#include "Cartidge.h"

#define P1		0xFF00
#define SB		0xFF01
#define SC		0xFF02
#define DIV		0xFF04
#define TIMA	0xFF05
#define TMA		0xFF06
#define TAC		0xFF07
#define IF		0xFF0F
#define NR10	0xFF10
#define NR11	0xFF11
#define NR12	0xFF12
#define NR13	0xFF13
#define NR14	0xFF14
#define NR21	0xFF16
#define NR22	0xFF17
#define NR23	0xFF18
#define NR24	0xFF19
#define NR30	0xFF1A
#define NR31	0xFF1A
#define NR32	0xFF1B
#define NR33	0xFF1C
#define NR34	0xFF1D
#define NR41	0xFF20
#define NR42	0xFF21
#define NR43	0xFF22
#define NR44	0xFF23
#define NR50	0xFF24
#define NR51	0xFF25
#define NR52	0xFF26
#define LCDC	0xFF40
#define STAT	0xFF41
#define SCY		0xFF42
#define SCX		0xFF43
#define LY		0xFF44
#define LYC		0xFF45
#define DMA		0xFF46
#define BGP		0xFF47
#define OBP0	0xFF48
#define OBP1	0xFF49
#define WY		0xFF4A
#define WX		0xFF4B
#define IE		0xFFFF

#define V_BLANK	Get_Bit(Read8(IE), 0)
#define LCDS	Get_Bit(Read8(IE), 1)
#define TIMER	Get_Bit(Read8(IE), 2)
#define SERIAL	Get_Bit(Read8(IE), 3)
#define JOYPAD	Get_Bit(Read8(IE), 4)
#define V_BLANK_FLAG	Get_Bit(Read8(IF), 0)
#define LCDS_FLAG		Get_Bit(Read8(IF), 1)
#define TIMER_FLAG		Get_Bit(Read8(IF), 2)
#define SERIAL_FLAG		Get_Bit(Read8(IF), 3)
#define JOYPAD_FLAG		Get_Bit(Read8(IF), 4)

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
	uint8 _IFlag;	//Interupt Flag (IE)
	Cartidge* _cartridge;

public:
	MMU(Cartidge* cartridge);
	~MMU();

private:
	void Set_Bios();

public:
	void BIOSLoaded(BOOL value);
	void Reset();

	uint8 MMU::Read8(uint16 address);
	uint16 MMU::Read16(uint16 address);
	void MMU::Write8(uint16 address, uint8 value);
	void MMU::Write16(uint16 address, uint16 value);
};
#endif // !MMU_H
