#include <stdio.h>
#include "CPU.h"
#include "Defines.h"

Cartidge* cartridge;
MMU* mmu;
CPU* cpu;

int test = 0;

void update(int i, int& ref) {
	ref = i;
}

int main()
{
	cartridge = new Cartidge("C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\EmulatorProjectChip-8\\ROMS\\SML.gb");
	mmu = new MMU(cartridge);
	cpu = new CPU(mmu);

	while (cpu->Run());

    return 0;
}

