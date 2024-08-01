#include "TerrainApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    TerrainApp ta(hInstance);

    if (!ta.Init())
        return 0;

    return ta.Run();
}