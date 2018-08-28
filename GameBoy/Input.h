#ifndef INPUT_H
#define INPUT_H

#ifdef __linux__ 
#elif _WIN32
#include <Windows.h>
#endif

class Input
{
public:
	Input();
	~Input();

	int IsExit();
};


#endif // !INPUT_H