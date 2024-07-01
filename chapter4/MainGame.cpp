#include "MainGame.h"

MainGame::MainGame(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

MainGame::~MainGame()
{
}

bool MainGame::Init()
{
    if (!D3DApp::Init())
        return false;

    return true;
}

void MainGame::OnResize()
{
    D3DApp::OnResize();
}

void MainGame::UpdateScene(float dt)
{
    
}

void MainGame::DrawScene()
{
    assert(md3dImmediateContext);
    assert(mSwapChain);

    const float customColor[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
    md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, customColor);
    md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    HR(mSwapChain->Present(0, 0));
}
