#include "InstancingAndCulling.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    InstancingAndCulling iac(hInstance);

    if (!iac.Init())
        return 0;

    return iac.Run();
}