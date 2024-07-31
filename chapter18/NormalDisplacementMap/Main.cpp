#include "NormalDisplacementMap.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    NormalDisplacementMap ndm(hInstance);

    if (!ndm.Init())
        return 0;

    return ndm.Run();
}