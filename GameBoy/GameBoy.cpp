#include <stdio.h>
#include "CPU.h"
#include "PPU.h"
#include "Defines.h"

Cartidge* cartridge;
MMU* mmu;
CPU* cpu;
PPU* ppu;

int test = 0;

char* t = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\t.gb";
char* ttt = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\ttt.gb";
char* cpuTestPath = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\cpu_instrs.gb";
char* cpuTestPath2 = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\oam_bug.gb";
char* rom = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\rom.bin";
char* gpu = "C:\\Users\\AdminUser.UserAdmin-PC\\Documents\\Visual Studio 2015\\Projects\\GameBoy\\Debug\\roms\\opus5.gb";

void update(int i, int& ref) {
	ref = i;
}

int main()
{
	cartridge = new Cartidge(gpu);
	mmu = new MMU(cartridge);
	cpu = new CPU(mmu);
	ppu = new PPU(cpu, mmu);
	

	while (cpu->Run())
	{
		ppu->Draw();
	}

    return 0;
}

