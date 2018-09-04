#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#pragma once

#include "Defines.h"
#include <iostream>
#include <fstream>
using namespace std;

#define ROM_SIZE 0X8000
#define VRAM_SIZE 0x2000
#define WRAM_SIZE 0x2000

class Cartidge
{
private:
	struct Details {
		BOOL nintendoBMP;
		char title[16];
		uint8 manufacturer[4];
		uint8 CGBFlag;
		uint16 licenseCode;
		uint8 SGBFlag;
		uint8 type;
		uint16 rom_size;
		uint8 rom_banks;
		uint8 ram_size;
		uint8 ram_banks;
		uint8 destinationCode;
		uint8 oldLicenseeCode;
		uint8 headerChecksum;
		BOOL isValid;
	};

	uint8* _ROM;	//Cartridge ROM
public:
	Details _details;

	Cartidge(char* path);
	~Cartidge();
private:
	void Load(char* path);

public:
	uint8 read(uint16 address);
	void write(uint16 address, uint8 value);
	uint8* Cartidge::Get_ROM();
};

#endif // !CARTRIDGE_H

