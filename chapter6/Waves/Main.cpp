#include "MainApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
	MainApp app(hInstance);

	if (!app.Init())
		return 0;

	return app.Run();
}
