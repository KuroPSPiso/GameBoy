#ifndef SPRITE_H
#define SPRITE_H

#include "Defines.h"
class Sprite
{
public:
	uint8 posY;
	uint8 posX;
	uint8 number;
	uint8 F; // OBJ-to-BG Priority | Y flip | X flip | Palette No | Tile VRAM Bank (CGB) | Palette No (CGB) | Palette No (CGB)

	Sprite();
	~Sprite();
	Sprite(uint8 yPosition, uint8 xPosition, uint8 TorPnumber, uint8 flag);
};
#endif 

