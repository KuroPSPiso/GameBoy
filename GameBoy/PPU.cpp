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
	uint8 scy	= _mmu->Read8(SCY);
	uint8 scx	= _mmu->Read8(SCX);
	uint8 wy	= _mmu->Read8(WY);
	uint8 wx	= _mmu->Read8(WX);
	uint8 line	= _mmu->Read8(LY);

	if (drawBG == TRUE)
	{
		uint16 mapOffset = (mapSelect == FALSE) ? 0x9800 : 0x9C00;
		uint16 tileOffset = (tileSelect == TRUE) ? 0x8000 : 0x8800;
		BOOL isUnsigned = tileSelect;
		uint8 tileSize = 0x10;
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
			posY = wx + line;
		}
		tileRow = ((uint8)(posY / 8)) * 32; //current ypos / 8 pixel rows per tile * 32 (max) tiles
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
			uint16 tileLocation = tileOffset + (tileSize * (tileNumber + (isUnsigned == TRUE)? 0 : 128));
			
			//TODO: why?
			tileColumnRemainder *= 2; 

			uint8 colourBit = posX % 8;
			colourBit -= 7;
			colourBit *= -1;
			//end why?

			uint8 tileUnMapped2 = _mmu->Read8(tileAddress + tileColumnRemainder + 1);
			uint8 tileUnMapped1 = _mmu->Read8(tileAddress + tileColumnRemainder + 0);
			uint8 pxColour = Get_Bit(tileUnMapped2, colourBit);
			pxColour <<= 1; //l-shift, set bit 1
			pxColour |= Get_Bit(tileUnMapped1, colourBit); //set bit 0

			if (line < 0x00 || line >= 0x90 || pixel < 0 || pixel >= 0xA0)
			{
				continue;
			}

			_display[pixel][line] = pxColour;
		}
		//Test draw full map
		/*CharArray* bgMap = new CharArray[0xFF];
		for (int x = 0; x < 0xFF; x++) //row no
		{
			bgMap[x].Clear();
			uint8 *columns = new uint8[0xFF];
			for (int y = 0; y < 0xFF; y+=0x01)
			{
				columns[x] = _mmu->Read8(mapOffset + y + (x * 0x100));
				for (int z = 0; z < 0xFF; z++)
				{
					bgMap[x].Append(SetPrintChar(' '));
				}
			}
			printf(bgMap[x].ToString());
			printf("\n");
		}*/
		/* (06/09/18)
		CharArray bgMap[0x100];
		for (uint8 y = 0x00; y < 0x1f; y += 0x01) {
			bgMap[(y * 0x08) + 0].Clear();
			bgMap[(y * 0x08) + 1].Clear();
			bgMap[(y * 0x08) + 2].Clear();
			bgMap[(y * 0x08) + 3].Clear();
			bgMap[(y * 0x08) + 4].Clear();
			bgMap[(y * 0x08) + 5].Clear();
			bgMap[(y * 0x08) + 6].Clear();
			bgMap[(y * 0x08) + 7].Clear();
			for (uint8 x = 0x00; x < 0xFF; x++) {
				uint16 column = _mmu->Read16(mapOffset + (x * 0x10));
				uint8 c0 = (column & 0x0003);
				uint8 c1 = (column & 0x000C) >> 2;
				uint8 c2 = (column & 0x0030) >> 4;
				uint8 c3 = (column & 0x00C0) >> 6;
				uint8 c4 = (column & 0x0300) >> 8;
				uint8 c5 = (column & 0x0C00) >> 10;
				uint8 c6 = (column & 0x3000) >> 12;
				uint8 c7 = (column & 0xC000) >> 14;
				bgMap[(y * 0x08) + 0].Append(SetPrintChar(c0));
				bgMap[(y * 0x08) + 1].Append(SetPrintChar(c1));
				bgMap[(y * 0x08) + 2].Append(SetPrintChar(c2));
				bgMap[(y * 0x08) + 3].Append(SetPrintChar(c3));
				bgMap[(y * 0x08) + 4].Append(SetPrintChar(c4));
				bgMap[(y * 0x08) + 5].Append(SetPrintChar(c5));
				bgMap[(y * 0x08) + 6].Append(SetPrintChar(c6));
				bgMap[(y * 0x08) + 7].Append(SetPrintChar(c7));
			}
		}*/
		/*
		for (int i = 0; i < 0xFF; i++)
		{
			if (bgMap != NULL)
			{
				if (bgMap[i].ToString() != NULL)
				{
					printf(bgMap[i].ToString());
				}
			}
		}*/
		/*
		ofstream mapfs("mapdata.txt", ofstream::out);
		for (int x = 0; x < 0x20 * 32; x++)
		{
			printf("blk %d:\n", x);
			mapfs << "blk " << x << ":\n";
			uint16 columns[8];

			for (int i = 0; i < 8; i++)
			{
				columns[i] = 0x00;
			}

			uint8 subAddress = _mmu->Read8(mapOffset + (x * 16));

			columns[0] = _mmu->Read16(8000 + subAddress);
			columns[1] = _mmu->Read16(8001 + subAddress);
			columns[2] = _mmu->Read16(8002 + subAddress);
			columns[3] = _mmu->Read16(8003 + subAddress);
			columns[4] = _mmu->Read16(8004 + subAddress);
			columns[5] = _mmu->Read16(8005 + subAddress);
			columns[6] = _mmu->Read16(8006 + subAddress);
			columns[7] = _mmu->Read16(8007 + subAddress);

			for (int z = 0; z <= 7; z++)
			{
				for (int y = 0; y <= 7; y++)
				{
					uint8 c0 = (columns[y] & 0x0003) >> 0;
					uint8 c1 = (columns[y] & 0x000C) >> 2;
					uint8 c2 = (columns[y] & 0x0030) >> 4;
					uint8 c3 = (columns[y] & 0x00C0) >> 6;
					uint8 c4 = (columns[y] & 0x0300) >> 8;
					uint8 c5 = (columns[y] & 0x0C00) >> 10;
					uint8 c6 = (columns[y] & 0x3000) >> 12;
					uint8 c7 = (columns[y] & 0xC000) >> 14;

					switch (z)
					{
					case 7:
						printf("%01x", c7);
						mapfs << +c7;
						//printf("%c", SetPrintChar(c3));
						break;
					case 6:
						printf("%01x", c6);
						mapfs << +c6;
						//printf("%c", SetPrintChar(c3));
						break;
					case 5:
						printf("%01x", c5);
						mapfs << +c5;
						//printf("%c", SetPrintChar(c3));
						break;
					case 4:
						printf("%01x", c4);
						mapfs << +c4;
						//printf("%c", SetPrintChar(c3));
						break;
					case 3:
						printf("%01x", c3);
						mapfs << +c3;
						//printf("%c", SetPrintChar(c3));
						break;
					case 2:
						printf("%01x", c2);
						mapfs << +c2;
						//printf("%c", SetPrintChar(c2));
						break;
					case 1:
						printf("%01x", c1);
						mapfs << +c1;
						//printf("%c", SetPrintChar(c1));
						break;
					case 0:
						printf("%01x", c0);
						mapfs << +c0;
						//printf("%c", SetPrintChar(c0));
						break;
					}
				}
				printf("\n");
				mapfs << "\n";
			}
		}
		mapfs.flush();
		mapfs.close();

		int i = 0;*/
	}
	
	if (drawWindow == TRUE)
	{
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

			//TODO: draw per row (but it's glitched)
			//PrintLine(row, FALSE);
			if (row >= 143)
			{
				Set_Bit(stat, 0, TRUE);			//0x01
				Set_Bit(stat, 1, FALSE);		//0x00
				_mmu->Write8(STAT, stat);		//0x01 = 1
				//printf("\t\tMODE-SWITCH-0-1");
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
				//Print();
				PrintLine(row, TRUE);
#if _WIN32
				COORD coord;
				coord.X = 0;
				coord.Y = 0;
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#elif __linux__
				printf("\033[1;1H");
#endif

				//printf("\r\r%2d", 144);
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
		////uint8 row = _mmu->Read8(LY) + 0x01;
		////_mmu->Write8(LY, row);
		if (_cpu->GetCycles() >= 169)
		{
			ScanLine();
			_cpu->ResetCycles();
			Set_Bit(stat, 0, FALSE);		//0x00
			Set_Bit(stat, 1, FALSE);		//0x00
			_mmu->Write8(STAT, stat);		//0x00 = 0
			//printf("\t\tMODE-SWITCH-3-0");
			////row = 0x00;
			////_mmu->Write8(LY, row);
			//PrintLine();
		}
		break;
	}

	Set_Bit(stat, 2, (_mmu->Read8(LY) == _mmu->Read8(LYC)));
	_mmu->Write8(STAT, stat);
}
