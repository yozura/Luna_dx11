#include "Pyramid.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Pyramid pyramid(hInstance);

    if (!pyramid.Init())
        return 0;

    return pyramid.Run();
}
