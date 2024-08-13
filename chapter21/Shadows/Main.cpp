#include "Shadows.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Shadows shadows(hInstance);

    if (!shadows.Init())
        return 0;

    return shadows.Run();
}w