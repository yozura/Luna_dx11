#include "ColoredBox.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int showCmd)
{
    ColoredBox coloredBox(hInstance);

    if (!coloredBox.Init())
        return 0;

    return coloredBox.Run();
}
