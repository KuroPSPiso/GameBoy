#include "PPU.h"

#define CLR3 0x01
#define CLR2 0x02
#define CLR1 0x03
#define CLR0 0x00

#define PXS3 '#'//'0'//"\xE2\x96\x91"
#define PXS2 'x'//'3'//"\xE2\x96\x92"
#define PXS1 '-'//'O'//"\xE2\x96\x93"
#define PXS0 ' '

class CharArray {
	struct {
		int length;
	};
	char* data;

private:
	void InitData(int lenght)
	{
		if (data != NULL) delete[] data;
		data = new char[0xFF];

		for (int i = 0; i < 0xFF; i++)
		{
			data[i] = NULL;
		}
	}

public:
	CharArray()
	{
		data = NULL;
		length = 0;
	}
	CharArray(int size)
	{
		length = size;
		InitData(0xFF);
	}
	~CharArray()
	{
	}

	void Append(char c) {
		if (data == NULL)
		{
			length = 1;
			data = new char[0x0001]{ c };
		}
		else
		{
			char* tempData = new char[0xFF];
			for (int i = 0; i < length; i++)
			{
				tempData[i] = data[i];
			}

			int strLenght = 1;
			int tempLength = length + strLenght;
			InitData(0xFF);
			for (int i = 0; i < length; i++)
			{
				data[i] = tempData[i];
			}
			delete tempData;
			length = tempLength;
			data[tempLength - strLenght] = c;
		}
	}
	void Append(char* str) {
		if (str == NULL) return;
		if (data == NULL)
		{
			length = strlen(str);
			data = str;
		}
		else
		{
			char* tempData = new char[0xFF];
			for (int i = 0; i < length; i++)
			{
				tempData[i] = data[i];
			}

			int strLenght = strlen(str);
			int tempLength = length + strLenght;
			InitData(0xFF);
			for (int i = 0; i < length; i++)
			{
				data[i] = tempData[i];
			}
			for (int i = 0; i < strLenght; i++)
			{
				data[tempLength - strLenght + i] = str[i];
			}
			delete tempData;
			length = tempLength;
		}
	}

	void Clear()
	{
		data = NULL;
		delete data;
		length = 0;
	}

	char* ToString()
	{
		return data;
	}
};

PPU::PPU(CPU* cpu, MMU* mmu)
{
	_cpu = cpu;
	_mmu = mmu;

	Reset();
}

PPU::~PPU()
{
}

char PPU::SetPrintChar(uint8 val)
{
	switch (val)
	{
	case CLR1:
		return PXS1;
	case CLR2:
		return PXS2;
	case CLR3:
		return PXS3;
	default:
		return PXS0;
	}
}

void PPU::Reset()
{
	for (int i = 0; i < 144; i++)
	{
		printf("\r");
	}
	for (int iDisplay = 0; iDisplay < 160 * 144; iDisplay++)
	{
		srand(iDisplay);
		switch (rand() % 4 + 1)
		{
		case CLR1:
			display[iDisplay] = CLR1;
			break;
		case CLR2:
			display[iDisplay] = CLR2;
			break;
		case CLR3:
			display[iDisplay] = CLR3;
			break;
		default:
			display[iDisplay] = CLR0;
			break;
		}
	}

	//Set tileset
	for (uint8 x = 0; x < 0xC0; x++) 
	{ 
		for (uint8 y = 0; y < 0x02; y++) 
		{ 
			for (uint8 z = 0; z < 0x0F; z+=2)
			{
				uint16 line = 0x0000;
				uint16 address = 0x8000 + z + (y * 2) + (x * 16);
				_mmu->Write16(address, line);
			}
		} 
	}

	Print();
}

CharArray *displayArray = new CharArray();

void PPU::Print()
{
	uint8 lineno = 0x00;
	printf("\n%X", lineno);
	for (int iDisplayY = 0; iDisplayY < 144 - 1; iDisplayY++)
	{
		for (int iDisplayX = 0; iDisplayX < 160 - 1; iDisplayX++)
		{
			if ((iDisplayX + (iDisplayY * 160)) < 0x5A00)
			{
				uint8 colour = display[iDisplayX + (iDisplayY * 160)];
				displayArray->Append(SetPrintChar(colour));
			}
			else
			{
				printf("_OOR_");
			}
		}
		printf("\n%X", iDisplayY);
		displayArray->Clear();
	}
}

void PPU::PrintLine()
{
	uint8 mapOffset = _mmu->Read8(BGP) ? 0x1C00 : 0x1800;
	mapOffset += ((_mmu->Read8(LY) + _mmu->Read8(SCY)) & 0xFF) >> 3;

	uint8 lineOffset = _mmu->Read8(SCY) >> 3;

	uint8 y = (_mmu->Read8(LY) + _mmu->Read8(SCY)) & 0x07;
	uint8 x = _mmu->Read8(SCX) & 0x07;
	uint8 canvasOffset = _mmu->Read8(LY) * 160 * 4;
	uint8 colour = 0x00;
	uint16 tileIndex = _mmu->Read8(mapOffset + lineOffset);
	if (Get_Bit(_mmu->Read8(LCDC), 4) && tileIndex < 128) tileIndex += 256;
	tileIndex += 0x8000;


	//CharArray *displayLine = new CharArray();
	for (int i = 0; i < 160/4; i++)
	{
		uint8 pxr = _mmu->Read8(tileIndex + y + x);
		display[canvasOffset + 0x0000] = (pxr & 0xF0) >> 6;
		display[canvasOffset + 0x0001] = (pxr & 0x30) >> 4;
		display[canvasOffset + 0x0002] = (pxr & 0x0F) >> 2;
		display[canvasOffset + 0x0003] = (pxr & 0x03);
		/*displayLine->Append(SetPrintChar(display[canvasOffset + 0x0003]));
		displayLine->Append(SetPrintChar(display[canvasOffset + 0x0002]));
		displayLine->Append(SetPrintChar(display[canvasOffset + 0x0001]));
		displayLine->Append(SetPrintChar(display[canvasOffset + 0x0000]));*/

		canvasOffset += 0x04;
		x++;
		if (x == 8)
		{
			x = 0;
			lineOffset = (lineOffset + 1) & 31;
			tileIndex = _mmu->Read8(mapOffset + lineOffset);
			if (Get_Bit(_mmu->Read8(LCDC), 4) && tileIndex < 128) tileIndex += 256;
			tileIndex += 0x8000;
		}
	}
	//if (displayLine->ToString() != NULL) {
	//	displayArray->Clear();
	//	displayArray->Append(displayLine->ToString());
	//	displayArray->Append("\n");
	//	//printf(displayLine->ToString());
	//	//printf("\n");
	//	displayLine->Clear();
	//}
	//delete displayLine;
}

void PPU::Draw()
{
	uint8 lcdc = _mmu->Read8(LCDC);

	if (Get_Bit(lcdc, 7) == FALSE) { return; } //lcd is off

	uint8 stat = _mmu->Read8(STAT);
	uint8 mode = (Get_Bit(stat, 1) << 1) + Get_Bit(stat, 0);
	switch (mode)
	{
	case MODE0:	//H-Blank
		if (_cpu->GetCycles() >= 201)
		{
			_cpu->ResetCycles();
			uint8 row = _mmu->Read8(LY) + 0x01;
			_mmu->Write8(LY, row);
			if (row >= 143)
			{
				Set_Bit(stat, 0, TRUE);			//0x01
				Set_Bit(stat, 1, FALSE);		//0x00
				_mmu->Write8(STAT, stat);		//0x01 = 1
				//printf("\t\tMODE-SWITCH-0-1");
			}
		}
		break;
	case MODE1:	//V-Blank
		if (_cpu->GetCycles() >= 456)
		{
			_cpu->ResetCycles();
			uint8 row = _mmu->Read8(LY) + 0x01;
			_mmu->Write8(LY, row);
			if (row >= 0x9A) //144 lines drawn + 10 vblank
			{
				row = 0x00;
				_mmu->Write8(LY, row);
				Set_Bit(stat, 0, FALSE);	//0x00
				Set_Bit(stat, 1, TRUE);		//0x10
				_mmu->Write8(STAT, stat);	//0x10 = 2
				Print();
				system("cls");
				//printf("\t\tMODE-SWITCH-1-2");
			}
		}
		break;
	case MODE2: //OAM reading
		if (_cpu->GetCycles() >= 77)
		{
			_cpu->ResetCycles();
			Set_Bit(stat, 0, TRUE);			//0x01
			Set_Bit(stat, 1, TRUE);			//0x11
			_mmu->Write8(STAT, stat);		//0x11 = 3
			//printf("\t\tMODE-SWITCH-2-3");
		}
		break;
	case MODE3: //OAM & VRAM reading
		if (_cpu->GetCycles() >= 169)
		{
			_cpu->ResetCycles();
			Set_Bit(stat, 0, FALSE);		//0x00
			Set_Bit(stat, 1, FALSE);		//0x00
			_mmu->Write8(STAT, stat);		//0x00 = 0
			//printf("\t\tMODE-SWITCH-3-0");
			PrintLine();
		}
		break;
	}
}
