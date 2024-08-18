#ifndef MESH_VIEW_H
#define MESH_VIEW_H

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
#include "Ssao.h"
#include "TextureMgr.h"
#include "BasicModel.h"

struct BoundingSphere
{
    BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
    DirectX::XMFLOAT3 Center;
    float Radius;
};

class MeshView : public D3DApp
{
public:
    MeshView(HINSTANCE hInstance);
    ~MeshView();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    void OnMouseDown(WPARAM btnState, int x, int y);
    void OnMouseUp(WPARAM btnState, int x, int y);
    void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void DrawSceneToSsaoNormalDepthMap();
    void DrawSceneToShadowMap();
    void DrawScreenQuad(ID3D11ShaderResourceView* srv);
    void BuildShadowTransform();
    void BuildScreenQuadGeometryBuffers();

private:
    TextureMgr mTexMgr;

    Sky* mSky;

    BasicModel* mTreeModel;
    BasicModel* mBaseModel;
    BasicModel* mStairsModel;
    BasicModel* mPillar1Model;
    BasicModel* mPillar2Model;
    BasicModel* mPillar3Model;
    BasicModel* mPillar4Model;
    BasicModel* mRockModel;

    std::vector<BasicModelInstance> mModelInstances;
    std::vector<BasicModelInstance> mAlphaClippedModelInstances;

    ID3D11Buffer* mSkySphereVB;
    ID3D11Buffer* mSkySphereIB;

    ID3D11Buffer* mScreenQuadVB;
    ID3D11Buffer* mScreenQuadIB;

    BoundingSphere mSceneBounds;

    static const int SMapSize = 2048;
    ShadowMap* mSmap;
    DirectX::XMFLOAT4X4 mLightView;
    DirectX::XMFLOAT4X4 mLightProj;
    DirectX::XMFLOAT4X4 mShadowTransform;

    Ssao* mSsao;

    float mLightRotationAngle;
    DirectX::XMFLOAT3 mOriginalLightDir[3];
    DirectionalLight mDirLights[3];

    Camera mCam;

    POINT mLastMousePos;
};

#endif /* MESH_VIEW_H */
