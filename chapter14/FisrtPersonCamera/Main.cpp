#include "FirstPersonCamera.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    FirstPersonCamera fpc(hInstance);

    if (!fpc.Init())
        return 0;

    return fpc.Run();
}