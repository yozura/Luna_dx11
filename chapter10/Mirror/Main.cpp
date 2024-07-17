#include "Mirror.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Mirror mirror(hInstance);

    if (!mirror.Init())
        return 0;

    return mirror.Run();
}