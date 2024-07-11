#include "LitSkull.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    LitSkull litSkull(hInstance);

    if (!litSkull.Init())
        return 0;

    return litSkull.Run();
}