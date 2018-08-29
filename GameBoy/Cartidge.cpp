#include "Cartidge.h"


Cartidge::Cartidge(char* path)
{
	Load(path);
}

void Cartidge::Load(char * path)
{
	ifstream file(path, ios::binary);
	if (!file.fail())
	{
		char* tempData = new char[0xFFFF];
		_ROM = new uint8[0xFFFF];
		uint16 byteCounter = 0x0000;
		/*while (file.eof() == FALSE)
		{
			tempData[byteCounter] = file.get();
			byteCounter++;
		}*/
		file.read(tempData, 0xFFFF);

		//Loop header
		uint8 bpm[] = {
			0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
			0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
			0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
		};
		uint8 iBPM = 0;
		uint8 iTitle = 0;
		uint8 iManufacturer = 0;
		uint8 xCheckSum = 0;

		_details.nintendoBMP = true;
		_details.isValid = FALSE;
		for (uint16 i = 0; i < 0xFFFF; i++)
		{
			uint8 byte = tempData[i];
			_ROM[i] = byte;
		}

		for (uint16 i = 0x0104; i <= 0x014D; i++)
		{
			uint8 byte = tempData[i];
			//_ROM[i] = byte;
			//Nintendo LOGO bitmap
			if (i >= 0x0104 && i <= 0x0133)
			{
				//TODO: Add logic for CGB support
				if (bpm[iBPM] != byte) //crash if bytes don't align
				{
					_details.nintendoBMP = false;
					return; 
				}
				iBPM++;
			}

			//Title
			else if (i >= 0x0134 && i <= 0x0143)
			{
				_details.title[iTitle] = byte;
				iTitle++;
				if (i >= 0x013F && i <= 0x0142)
				{
					_details.manufacturer[iManufacturer] = byte;
					iManufacturer++;
				}
				else if (i == 0x0143)
				{
					_details.CGBFlag = byte;
				}
			}

			//type
			else if (i == 0x0147)
			{
				if (i == 0x00)
				{
					_details.type = 0x00;
				}
				else
				{
					return; //not supported;
				}
			}

			//rom size
			else if (i == 0x0148)
			{
				if (byte <= 0x08)
				{
					_details.rom_size = 32 * (byte + 1);
					_details.rom_banks = 4 * (byte + 1);
				}
				switch (byte)
				{
				case 0x05:
					_details.rom_banks = 0;
					break;
				case 0x06:
					_details.rom_banks = 0;
					break;
				}
			}

			//ram size
			else if (i == 0x0149)
			{
				switch (byte)
				{
				case 0x01:
					_details.ram_size = 0x0800;
					_details.ram_banks = 0;
					break;
				case 0x02:
					_details.ram_size = 0x2000;
					_details.ram_banks = 0;
					break;
				case 0x03:
					_details.ram_size = 0x8000;
					_details.ram_banks = 4;
					break;
				case 0x04:
					_details.ram_size = 0x20000;
					_details.ram_banks = 16;
					break;
				case 0x05:
					_details.ram_size = 0x10000;
					_details.ram_banks = 8;
					break;
				case 0x00:
				default:
					_details.ram_size = 0;
					_details.ram_banks = 0;
					break;
				}
			}

			//destination code
			else if (i == 0x014A)
			{
				_details.destinationCode = byte;
			}

			//checksum
			if (i >= 0x0134 && i <= 0x014C)
			{
				xCheckSum = xCheckSum - tempData[i] - 1;
			}

			if (i == 0x014D)
			{
				if (xCheckSum != byte) 
				{
					return; // checksum failed;
				}
				else
				{
					_details.headerChecksum = xCheckSum;
					_details.isValid = TRUE;
				}
			}
		}
	}
}

Cartidge::~Cartidge()
{
	delete[] _ROM;
}

uint8 Cartidge::read(uint16 address)
{
	return _ROM[address];
}

void Cartidge::write(uint16 address, uint8 value)
{
	_ROM[address] = value;
}

uint8* Cartidge::Get_ROM()
{
	return _ROM;
}