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

public:
	Input();
	~Input();

	uint8 GetController();

	int HasInput();
	int IsExit();
};


#endif // !INPUT_H