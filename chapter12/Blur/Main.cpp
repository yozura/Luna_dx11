#include "Blur.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Blur blur(hInstance);

    if (!blur.Init())
        return 0;

    return blur.Run();
}