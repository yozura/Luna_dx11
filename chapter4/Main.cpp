#include "MainGame.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    MainGame mainGame(hInstance);

    if (!mainGame.Init())
        return 0;

    return mainGame.Run();
}
