#include "DynamicCubeMap.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    DynamicCubeMap dcm(hInstance);

    if (!dcm.Init())
        return 0;

    return dcm.Run();
}