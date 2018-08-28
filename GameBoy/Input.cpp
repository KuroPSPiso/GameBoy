#include "Input.h"


Input::Input()
{
}

Input::~Input()
{
}

int Input::IsExit()
{
#if _WIN32
	return GetAsyncKeyState(VK_ESCAPE) < 0;
#elif _linux_
	return 1;
#endif
}
