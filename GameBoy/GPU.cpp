#include "GPU.h"

#define MODE0 0x00
#define MODE1 0x01
#define MODE2 0x02
#define MODE3 0x03

GPU::GPU(CPU* cpu, MMU* mmu)
{
	_cpu = cpu;
	_mmu = mmu;
}


GPU::~GPU()
{
}

void GPU::Draw()
{
	uint8 lcdc = _mmu->Read8(LCDC);

	if (Get_Bit(lcdc, 7) == FALSE) { return; }

	uint8 stat = _mmu->Read8(STAT);
	uint8 mode = (Get_Bit(stat, 1) < 1) + Get_Bit(stat, 0);
	switch (mode)
	{
	case MODE0:

		break;
	case MODE1:
	case MODE2:
	case MODE3:
		break;
	}
}
