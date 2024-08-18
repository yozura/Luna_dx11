#include "MeshView.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    MeshView mv(hInstance);

    if (!mv.Init())
        return 0;

    return mv.Run();
}