#ifndef PPU_H
#define PPU_H

#include "Defines.h"
#include "Sprite.h"
#include "CPU.h"

#define MODE0 0x00	//H-Blank
#define MODE1 0x01	//V-Blank
#define MODE2 0x02	//OAM-RAM
#define MODE3 0x03	//LCD setting

class PPU
{

	uint8 display[0x5A00];
	uint8 _display[0xA0][0x90];
	CPU* _cpu;
	MMU* _mmu;

	char PPU::SetPrintChar(uint8 val);

public:
	PPU(CPU* cpu, MMU* mmu);
	~PPU();

	void PPU::Reset();
	void PPU::ResetTileSet();
	void PPU::UpdateTileSet();
	void PPU::Print();
	void PPU::PrintLine(uint8 row, BOOL printAll = FALSE);
	void PPU::ScanLine();
	Sprite * GetSprites(uint8 & lenght, uint8 x, uint8 y);
	void PPU::Draw();
};

#endif // !PPU_H