#include "PickingApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    PickingApp pa(hInstance);

    if (!pa.Init())
        return 0;

    return pa.Run();
}