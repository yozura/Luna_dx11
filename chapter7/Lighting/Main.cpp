#include "Lighting.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Lighting lighting(hInstance);

    if (!lighting.Init())
        return 0;

    return lighting.Run();
}