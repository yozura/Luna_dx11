#include "VecAdd.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    VecAdd va(hInstance);

    if (!va.Init())
        return 0;

    va.DoComputeWork();

    return 0;
}