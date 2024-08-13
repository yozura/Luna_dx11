#ifndef SHADOWS_H
#define SHADOWS_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "RenderStates.h"
#include "ShadowMap.h"

enum RenderOptions
{
    RenderOptionsBasic = 0,
    RenderOptionsNormalMap = 1,
    RenderOptionsDisplacementMap = 2
};

struct BoundingSphere
{
    BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
    DirectX::XMFLOAT3 Center;
    float Radius;
};

class Shadows : public D3DApp
{
public:
    Shadows(HINSTANCE hInstance);
    ~Shadows();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void DrawSceneToShadowMap();
    void DrawScreenQuad();
    void BuildShadowTransform();
    void BuildShapeGeometryBuffers();
    void BuildSkullGeometryBuffers();
    void BuildScreenQuadGeometryBuffers();

private:
    Sky* mSky;

    ID3D11Buffer* mShapesVB;
    ID3D11Buffer* mShapesIB;

    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;

    ID3D11Buffer* mSkySphereVB;
    ID3D11Buffer* mSkySphereIB;

    ID3D11Buffer* mScreenQuadVB;
    ID3D11Buffer* mScreenQuadIB;

    ID3D11ShaderResourceView* mStoneTexSRV;
    ID3D11ShaderResourceView* mBrickTexSRV;

    ID3D11ShaderResourceView* mStoneNormalTexSRV;
    ID3D11ShaderResourceView* mBrickNormalTexSRV;

    BoundingSphere mSceneBounds;

    static const int SMapSize = 2048;
    ShadowMap* mSmap;
    DirectX::XMFLOAT4X4 mLightView;
    DirectX::XMFLOAT4X4 mLightProj;
    DirectX::XMFLOAT4X4 mShadowTransform;

    float mLightRotationAngle;
    DirectX::XMFLOAT3 mOriginalLightDir[3];
    DirectionalLight mDirLights[3];
    Material mGridMat;
    Material mBoxMat;
    Material mCylinderMat;
    Material mSphereMat;
    Material mSkullMat;

    // Define transformations from local spaces to world space.
    DirectX::XMFLOAT4X4 mSphereWorld[10];
    DirectX::XMFLOAT4X4 mCylWorld[10];
    DirectX::XMFLOAT4X4 mBoxWorld;
    DirectX::XMFLOAT4X4 mGridWorld;
    DirectX::XMFLOAT4X4 mSkullWorld;

    int mBoxVertexOffset;
    int mGridVertexOffset;
    int mSphereVertexOffset;
    int mCylinderVertexOffset;

    UINT mBoxIndexOffset;
    UINT mGridIndexOffset;
    UINT mSphereIndexOffset;
    UINT mCylinderIndexOffset;

    UINT mBoxIndexCount;
    UINT mGridIndexCount;
    UINT mSphereIndexCount;
    UINT mCylinderIndexCount;

    UINT mSkullIndexCount;

    RenderOptions mRenderOptions;

    Camera mCam;

    POINT mLastMousePos;
};

#endif /* SHADOWS_H */
