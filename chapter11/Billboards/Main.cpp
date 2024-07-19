#include "Billboards.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    Billboards billboards(hInstance);

    if (!billboards.Init())
        return 0;

    return billboards.Run();
}