#include "Hills.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Hills Hills(hInstance);

    if (!Hills.Init())
        return 0;

    return Hills.Run();
}
