#ifndef PICKING_H
#define PICKING_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "RenderStates.h"

class PickingApp : public D3DApp
{
public:
    PickingApp(HINSTANCE hInstance);
    ~PickingApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildMeshGeometryBuffers();
    void Pick(int sx, int sy);
    bool Slaps(DirectX::XMVECTOR rayOrigin,
               DirectX::XMVECTOR rayDir,
               DirectX::XMVECTOR boxMin,
               DirectX::XMVECTOR boxMax,
               float& tMin);

private:
    ID3D11Buffer* mMeshVB;
    ID3D11Buffer* mMeshIB;

    std::vector<Vertex::Basic32> mMeshVertices;
    std::vector<UINT> mMeshIndices;

    DirectX::BoundingBox mMeshBox;

    DirectX::XMVECTOR mMeshBoxMin;
    DirectX::XMVECTOR mMeshBoxMax;

	DirectionalLight mDirLights[3];
	Material mMeshMat;
    Material mPickedTriangleMat;

	DirectX::XMFLOAT4X4 mMeshWorld;

    UINT mMeshIndexCount;
    UINT mPickedTriangle;

    Camera mCam;

	POINT mLastMousePos;
};

#endif /* PICKING_H */
