#include "BezierTessellation.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    BezierTessellation bt(hInstance);

    if (!bt.Init())
        return 0;

    return bt.Run();
}