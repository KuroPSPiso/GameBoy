#include "Input.h"


#define EXIT_KEY VK_ESCAPE

#define A_KEY 0x5A
#define B_KEY 0x58
#define START_KEY VK_RETURN
#define SELECT_KEY VK_RSHIFT
#define UP_KEY VK_UP
#define DOWN_KEY VK_DOWN
#define LEFT_KEY VK_LEFT
#define RIGHT_KEY VK_RIGHT

Input::Input()
{
}

Input::~Input()
{
}

uint8 Input::GetController()
{
	return _controller;
}

int Input::HasInput()
{
#if _WIN32
	BOOL hasInput = FALSE;

	if (GetAsyncKeyState(EXIT_KEY) < 0)
	{
		hasInput = TRUE;
	}

	if (GetAsyncKeyState(RIGHT_KEY) < 0)
	{
		Set_Bit(_controller, 0, TRUE);
		hasInput = TRUE;
	}
	if (GetAsyncKeyState(LEFT_KEY) < 0)
	{
		Set_Bit(_controller, 1, TRUE);
		hasInput = TRUE;
	}
	if (GetAsyncKeyState(UP_KEY) < 0)
	{
		Set_Bit(_controller, 2, TRUE);
		hasInput = TRUE;
	}
	if (GetAsyncKeyState(DOWN_KEY) < 0)
	{
		Set_Bit(_controller, 3, TRUE);
		hasInput = TRUE;
	}
	if (GetAsyncKeyState(A_KEY) < 0)
	{
		Set_Bit(_controller, 4, TRUE);
		hasInput = TRUE;
	}
	if (GetAsyncKeyState(B_KEY) < 0)
	{
		Set_Bit(_controller, 5, TRUE);
		hasInput = TRUE;
	}
	if (GetAsyncKeyState(SELECT_KEY) < 0)
	{
		Set_Bit(_controller, 6, TRUE);
		hasInput = TRUE;
	}
	if (GetAsyncKeyState(START_KEY) < 0)
	{
		Set_Bit(_controller, 7, TRUE);
		hasInput = TRUE;
	}

	return hasInput;
#elif _linux_
	return 0;
#endif
}

int Input::IsExit()
{
#if _WIN32
	return GetAsyncKeyState(EXIT_KEY) < 0;
#elif _linux_
	return 1;
#endif
}
