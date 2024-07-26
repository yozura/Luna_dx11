#ifndef LIT_SKULL_H
#define LIT_SKULL_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"

#include <DirectXCollision.h>

struct InstancedData
{
    DirectX::XMFLOAT4X4 World;
    DirectX::XMFLOAT4   Color;
};

class InstancingAndCulling : public D3DApp
{
public:
    InstancingAndCulling(HINSTANCE hInstance);
    ~InstancingAndCulling();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildSkullGeometryBuffers();
    void BuildInstancedBuffer();

private:
    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;
    ID3D11Buffer* mInstancedBuffer;

    DirectX::BoundingSphere  mSkullSphere;
    DirectX::BoundingFrustum mCamFrustum;

    UINT mVisibleObjectCount;

    std::vector<InstancedData> mInstancedData;

    bool mFrustumCullingEnabled;

	DirectionalLight mDirLights[3];
	Material mSkullMat;
	
    DirectX::XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;

    Camera mCam;

	POINT mLastMousePos;
};

#endif /* LIT_SKULL_H */
