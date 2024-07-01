#ifndef MAINGAME_H
#define MAINGAME_H

#include "D3DApp.h"

class MainGame : public D3DApp
{
public:
    MainGame(HINSTANCE hInstance);
    ~MainGame();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();
};

#endif /* MAINGAME_H */