#include "Shapes.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,	PSTR cmdLine, int showCmd)
{
	Shapes shapes(hInstance);

	if (!shapes.Init())
		return 0;

	return shapes.Run();
}
