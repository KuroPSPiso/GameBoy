#ifndef INPUT_H
#define INPUT_H

#ifdef __linux__ 
#elif _WIN32
#include <Windows.h>
#endif
#include "Defines.h"

class Input
{
	uint8 _controller;
	uint8 _keyRow;

public:
	Input();
	~Input();

	uint8 GetController();

	int HasInput();
	BOOL IsExit();

	uint8 Read8();
	void Write8(uint8 value);
};


#endif // !INPUT_H