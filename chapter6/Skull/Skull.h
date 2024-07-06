#ifndef SKULL_H
#define SKULL_H

#include "D3DApp.h"
#include "GeometryGenerator.h"

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

class Skull : public D3DApp
{
public:
    Skull(HINSTANCE hInstance);
    ~Skull();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);
    
private:
    void BuildGeometryBuffers();
    void BuildFX();
    void BuildVertexLayout();

private:
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;

    ID3DX11Effect* mFX;
    ID3DX11EffectTechnique* mTech;
    ID3DX11EffectMatrixVariable* mfxWorldViewProj;

    ID3D11InputLayout* mInputLayout;

    ID3D11RasterizerState* mWireFrameRS;

    XMFLOAT4X4 mSkullWorld;

    UINT mSkullIndexCount;

    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

    float mTheta;
    float mPhi;
    float mRadius;

    POINT mLastMousePos;
};

#endif

