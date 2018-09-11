#include "Sprite.h"


Sprite::Sprite()
{
}


Sprite::~Sprite()
{
}

Sprite::Sprite(uint8 yPosition, uint8 xPosition, uint8 TorPnumber, uint8 flag)
{
	posY = yPosition;
	posX = xPosition;
	number = TorPnumber;
	F = flag;
}