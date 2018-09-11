#include "PPU.h"

#define CLR3 0x00
#define CLR2 0x03
#define CLR1 0x02
#define CLR0 0x01

#define PXS3 '#'//'0'//"\xE2\x96\x91"
#define PXS2 'x'//'3'//"\xE2\x96\x92"
#define PXS1 '-'//'O'//"\xE2\x96\x93"
#define PXS0 ' '

class CharArray {
	int length;
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

	int Length() { return length; }

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
	for (int y = 0; y < 144; y++)
	{
		for (int x = 0; x < 160; x++)
		{
			//srand(iDisplay);
			int random = rand() % 4 + 1;
			switch (0x02)
			{
			case CLR1:
				_display[x][y] = CLR1;
				break;
			case CLR2:
				_display[x][y] = CLR2;
				break;
			case CLR3:
				_display[x][y] = CLR3;
				break;
			default:
				_display[x][y] = CLR0;
				break;
			}
		}
	}

	ResetTileSet();
	UpdateTileSet();
	//Print();
}

CharArray *displayArray = new CharArray();

void PPU::ResetTileSet()
{
	//Set tileset
	for (uint8 x = 0; x < 0xC0; x++)
	{
		for (uint8 y = 0; y < 0x02; y++)
		{
			for (uint8 z = 0; z < 0x0F; z += 2)
			{
				uint16 line = 0x0000;
				uint16 address = 0x8000 + z + (y * 2) + (x * 16);
				_mmu->Write16(address, line);
			}
		}
	}
}

void PPU::UpdateTileSet()
{

}

void PPU::Print()
{
	/*
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
	}*/
	CharArray displayLine;
	for (uint8 y = 0x00; y < 0x90; y++)
	{
		displayLine.Clear();
		displayLine.Append('|');
		for (uint8 x = 0x00; x < 0xA0; x++)
		{
			displayLine.Append(SetPrintChar(_display[x][y]));
		}
		displayLine.Append('|');
		displayLine.Append('\n');
		printf(displayLine.ToString());
		//printf("\n");
	}
	printf("display initialized");
}

void PPU::PrintLine(uint8 row, BOOL printAll)
{
	CharArray displayLine;
	if (printAll == TRUE)
	{
		/*
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

		*/

		ofstream vramfs("VRAM.txt", ofstream::out);
		for (int i = 0; i < 0x2000; i++)
		{
			vramfs << _mmu->Read8(i + 0x8000);
		}
		vramfs.flush();
		vramfs.close();

		ofstream mapfs("tiles.txt", ofstream::out);


		mapfs << "tile set 0 8000-8FFF \n";
		for (uint16 f = 0x0000; f <= 0x8FFF - 0x8000; f++)
		{
			mapfs << _mmu->Read8(f + 0x8000);
		}

		mapfs << "\n";

		mapfs << "tile set 0 9000-97FF \n";
		for (uint16 f = 0x0000; f <= 0x97FF - 0x9000; f++)
		{
			mapfs << _mmu->Read8(f + 0x9000);
		}

		mapfs << "\n";
		mapfs << "tile map 0 9800-9BFF \n";
		for (uint16 f = 0x0000; f <= 0x9BFF - 0x9800; f++)
		{
			mapfs << _mmu->Read8(f + 0x9800);
		}

		mapfs << "\n";
		mapfs << "tile map 1 9C00-9FFF \n";
		for (uint16 f = 0x0000; f <= 0x9FFF - 0x9C00; f++)
		{
			mapfs << _mmu->Read8(f + 0x9C00);
		}
		mapfs.flush();
		mapfs.close();

		char* charArray = new char[0x90 * 0xA1];
		for (uint8 y = 0x00; y < 0x90; y++)
		{
			displayLine.Clear();
			displayLine.Append('|');
			for (uint8 x = 0x00; x < 0xA0; x++)
			{
				displayLine.Append(SetPrintChar(_display[x][y]));
			}
			displayLine.Append('|');
			displayLine.Append('\n');
			printf(displayLine.ToString());
		}
		printf("display rendered");
	}
	else
	{
		displayLine.Clear();
		displayLine.Append('|');
		for (uint8 x = 0x00; x < 0xA0; x++)
		{
			displayLine.Append(SetPrintChar(_display[x][row]));
		}
		displayLine.Append('|');
		displayLine.Append('\n');
		printf(displayLine.ToString());

	}
}

void PPU::ScanLine()
{
	uint8 lcdc = _mmu->Read8(LCDC);

	BOOL drawBG = Get_Bit(lcdc, 0);
	BOOL objSize = Get_Bit(lcdc, 2);
	BOOL mapSelect = Get_Bit(lcdc, 3);
	BOOL tileSelect = Get_Bit(lcdc, 4);
	BOOL drawWindow = Get_Bit(lcdc, 5);
	uint8 scy = _mmu->Read8(SCY);
	uint8 scx = _mmu->Read8(SCX);
	uint8 wy = _mmu->Read8(WY);
	uint8 wx = _mmu->Read8(WX);
	uint8 line = _mmu->Read8(LY);
	uint8 tileSize = 0x08;

	//Draw background
	if (drawBG == TRUE)
	{
		uint16 mapOffset = (mapSelect == FALSE) ? 0x9800 : 0x9C00;
		uint16 tileOffset = (tileSelect == TRUE) ? 0x8000 : 0x8800;
		BOOL isUnsigned = tileSelect;
		uint8 tileSize = 0x08;
		uint8 posY = 0x00;
		uint16 tileRow = 0x0000;
		uint8 tileRowRemainder = 0x00;
		uint16 tileColumn = 0x0000;
		uint8 tileColumnRemainder = 0x00;
		uint16 tileAddress = 0x0000;
		int tileNumber = 0;
		if (drawWindow == FALSE)
		{
			posY = scy + line;
		}
		else
		{
			posY = wy + line;
		}
		tileRow = ((uint8)(posY / 8) * 32); //current ypos / 8 pixel rows per tile * 32 (max) tiles
		tileRowRemainder = (posY % 8);

		for (int pixel = 0; pixel < 160; pixel++)
		{
			uint8 posX = pixel + scx;

			if (drawWindow == TRUE)
			{
				if (posX >= wx)
				{
					posX = pixel - wx;
				}
			}
			tileColumn = (posX / 8); //current ypos / 8 pixel columns per tile
			tileColumnRemainder = (posX % 8);

			tileAddress = mapOffset + tileRow + tileColumn;
			if (isUnsigned == TRUE)
			{
				tileNumber = _mmu->Read8(tileAddress);
			}
			else
			{
				tileNumber = (signed int)_mmu->Read8(tileAddress);
			}
			uint16 tileLocation = tileOffset; // +(16 * (tileNumber)); //+ (isUnsigned == TRUE) ? 0 : 128));
			tileLocation += 16 * tileNumber;
			
			BYTE line2 = posY % 8;
			line2 *= 2; // each vertical line takes up two bytes of memory
			BYTE data1 = _mmu->Read8(tileLocation + line2);
			BYTE data2 = _mmu->Read8(tileLocation + line2 + 1);

			int colourBit = posX % 8;
			colourBit -= 7;
			colourBit *= -1;

			uint8 pxColour = Get_Bit(data2, colourBit);
			pxColour <<= 1;
			pxColour |= Get_Bit(data1, colourBit);

			if (line < 0x00 || line >= 0x90)
			{
				continue; //skip
			}

			_display[pixel][line] = pxColour;
		}
	}

	//Draw tiles
	if (drawWindow == TRUE)
	{
		tileSize = (Get_Bit(lcdc, 2) == TRUE) ? 8 : 16;

		uint8 posY = wy + line;

		uint8 spriteCount = 0;
		Sprite* sprites = GetSprites(spriteCount, wx, posY);

		if (spriteCount == 0) return;

		if (tileSelect == TRUE) //unsigned map 1 (0x8000-0x8FFF)
		{
			uint16 tile = 0x00; //blank

		}
		else //signed map 2 (0x8800-0x97FF)
		{
			uint16 tile = 0x00; //blank
		}
	}
}

Sprite* PPU::GetSprites(uint8& lenght, uint8 x, uint8 y)
{
	Sprite data[40];
	lenght = 0;

	for (int spriteIndex = 0; spriteIndex < 40; spriteIndex++)
	{
		if (lenght >= 10) { break; } //max 10 sprites per line
		Sprite tempSprite;
		for (int dataIndex = 0; dataIndex < 4; dataIndex++)
		{
			uint8 dataByte = _mmu->Read8(0xFE00 + (spriteIndex * 4) + dataIndex);
			switch(dataIndex)
			{
			case 3:
				tempSprite.F = dataByte;
				break;
			case 2:
				tempSprite.number = dataByte;
				break;
			case 1:
				tempSprite.posX = dataByte;
				break;
			case 0:
				tempSprite.posY = dataByte;
				break;
			}
		}

		//TODO: wrap around x and y
		if (
			tempSprite.posX >= x &&
			tempSprite.posX <= x + 160 && 
			y >= tempSprite.posY &&
			y < tempSprite.posY + 8
			)
		{
			data[lenght] = tempSprite;
			lenght++;
		}
	}

	return data;
}

void PPU::Draw()
{
	uint8 lcdc = _mmu->Read8(LCDC);
	uint8 stat = _mmu->Read8(STAT);
	uint8 row = _mmu->Read8(LY);

	if (Get_Bit(lcdc, 7) == FALSE) //lcd is off
	{ 
		stat &= 252;
		Set_Bit(stat, 0, 1);
		_mmu->Write8(STAT, stat);
		return;
	} 

	uint8 mode = (Get_Bit(stat, 1) << 1) + Get_Bit(stat, 0);
	switch (mode)
	{
	case MODE0:	//H-Blank
		if (_cpu->GetCycles() >= 201)
		{
			_cpu->ResetCycles();
			row = _mmu->Read8(LY) + 0x01;
			_mmu->Write8(LY, row);

			if (row >= 143)
			{
				Set_Bit(stat, 0, TRUE);			//0x01
				Set_Bit(stat, 1, FALSE);		//0x00
				_mmu->Write8(STAT, stat);		//0x01 = 1
			}
			else
			{
				Set_Bit(stat, 0, FALSE);		//0x00
				Set_Bit(stat, 1, TRUE);			//0x10
				_mmu->Write8(STAT, stat);		//0x10 = 2
			}
		}
		break;
	case MODE1:	//V-Blank
		if (_cpu->GetCycles() >= 456)
		{
			_cpu->ResetCycles();
			row += 0x01;
			_mmu->Write8(LY, row);
			Set_Bit(stat, 0, FALSE);	//0x00
			Set_Bit(stat, 1, TRUE);		//0x10

			if (row >= 0x9A) //144 lines drawn + 10 vblank
			{
				row = 0x00;
				_mmu->Write8(LY, row);
				_mmu->Write8(STAT, stat);	//0x10 = 2
				PrintLine(row, TRUE);
#if _WIN32
				COORD coord;
				coord.X = 0;
				coord.Y = 0;
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#elif __linux__
				printf("\033[1;1H");
#endif
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
		}
		break;
	case MODE3: //OAM & VRAM reading
		if (_cpu->GetCycles() >= 169)
		{
			ScanLine();
			_cpu->ResetCycles();
			Set_Bit(stat, 0, FALSE);		//0x00
			Set_Bit(stat, 1, FALSE);		//0x00
			_mmu->Write8(STAT, stat);		//0x00 = 0
		}
		break;
	}

	Set_Bit(stat, 2, (_mmu->Read8(LY) == _mmu->Read8(LYC)));
	_mmu->Write8(STAT, stat);
}
