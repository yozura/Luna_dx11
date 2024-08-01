#include "CubeMap.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    CubeMap cm(hInstance);

    if (!cm.Init())
        return 0;

    return cm.Run();
}