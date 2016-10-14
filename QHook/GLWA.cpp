#include "GLWA.h"

glwa GetLayeredWindowAttributesEx;

void LoadGLWA()
{
	//Load and use GLWA only if XP...
	HINSTANCE hLibUser32 = LoadLibrary("user32.dll");
	//If older than XP, this will be NULL
	GetLayeredWindowAttributesEx = (glwa)GetProcAddress(hLibUser32,"GetLayeredWindowAttributes");
}