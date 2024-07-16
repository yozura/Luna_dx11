#include "Blend.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Blend blend(hInstance);

    if (!blend.Init())
        return 0;

    return blend.Run();
}