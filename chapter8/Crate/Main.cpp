#include "Crate.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Crate crate(hInstance);

    if (!crate.Init())
        return 0;

    return crate.Run();
}