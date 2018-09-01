#ifndef GPU_H
#define GPU_H

#include "Defines.h"
#include "CPU.h"

class GPU
{

	uint16 display[160*144];
	CPU* _cpu;
	MMU* _mmu;

public:
	GPU(CPU* cpu, MMU* mmu);
	~GPU();

	void Draw();
};

#endif // !GPU_H