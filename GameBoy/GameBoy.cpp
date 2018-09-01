#include <stdio.h>
#include "CPU.h"
#include "GPU.h"
#include "Defines.h"

Cartidge* cartridge;
MMU* mmu;
CPU* cpu;
GPU* gpu;

int test = 0;

char* tPath = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\t.gb";
char* cpuTestPath = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\cpu_instrs.gb";
char* cpuTestPath2 = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\oam_bug.gb";
char* rom = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\rom.bin";

void update(int i, int& ref) {
	ref = i;
}

int main()
{
	cartridge = new Cartidge(tPath);
	mmu = new MMU(cartridge);
	cpu = new CPU(mmu);
	gpu = new GPU(cpu, mmu);

	while (cpu->Run())
	{
		gpu->Draw();
	}

    return 0;
}

