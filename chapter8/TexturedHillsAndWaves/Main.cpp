#include "TexturedHillsAndWaves.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    TexturedHillsAndWaves thw(hInstance);

    if (!thw.Init())
        return 0;

    return thw.Run();
}