#include "Box.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Box Box(hInstance);

    if (!Box.Init())
        return 0;

    return Box.Run();
}
