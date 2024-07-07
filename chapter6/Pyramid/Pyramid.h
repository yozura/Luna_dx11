#ifndef PYRAMID_H
#define PYRAMID_H

#include "MathHelper.h"
#include "D3DApp.h"

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

class Pyramid : public D3DApp
{
public:
    Pyramid(HINSTANCE hInstance);
    ~Pyramid();

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
    ID3D11Buffer* mPyramidPosVertexBuffer;
    ID3D11Buffer* mPyramidColorVertexBuffer;
    ID3D11Buffer* mPyramidIndexBuffer;
    
    ID3DX11Effect* mFX;
    ID3DX11EffectTechnique* mTech;
    ID3DX11EffectMatrixVariable* mfxWorldViewProj;

    ID3D11InputLayout* mInputLayout;

    ID3D11RasterizerState* mWireFrameRS;

    XMFLOAT4X4 mWorld;
    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

    UINT mPyramidIndexCount;

    float mTheta;
    float mPhi;
    float mRadius;

    POINT mLastMousePos;
};

#endif /* PYRAMID_H */