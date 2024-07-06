#include "Skull.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
	Skull skull(hInstance);

	if (!skull.Init())
		return 0;

	return skull.Run();
}
