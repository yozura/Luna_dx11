#ifndef DYNAMIC_CUBE_MAP_H
#define DYNAMIC_CUBE_MAP_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"

class DynamicCubeMap : public D3DApp
{
public:
    DynamicCubeMap(HINSTANCE hInstance);
    ~DynamicCubeMap();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void DrawScene(const Camera& camera, bool drawCenterSphere);
    void BuildCubeFaceFrame(float x, float y, float z);
    void BuildDynamicCubeMapView();
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();

private:
    Sky* mSky;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

    ID3D11ShaderResourceView* mFloorTexSRV;
    ID3D11ShaderResourceView* mStoneTexSRV;
    ID3D11ShaderResourceView* mBrickTexSRV;

    ID3D11DepthStencilView*   mDynamicCubeMapDSV;
    ID3D11RenderTargetView*   mDynamicCubeMapRTV[6];
    ID3D11ShaderResourceView* mDynamicCubeMapSRV;
    
    D3D11_VIEWPORT mCubeMapViewport;
	
    static const int CubeMapSize = 256;

	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;
    Material mCenterSphereMat;

	DirectX::XMFLOAT4X4 mSphereWorld[10];
	DirectX::XMFLOAT4X4 mCylinderWorld[10];
	DirectX::XMFLOAT4X4 mBoxWorld;
	DirectX::XMFLOAT4X4 mGridWorld;
	DirectX::XMFLOAT4X4 mSkullWorld;
	DirectX::XMFLOAT4X4 mCenterSphereWorld;

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

	UINT mLightCount;

    Camera mCam;
    Camera mCubeMapCamera[6];

	POINT mLastMousePos;
};

#endif /* DYNAMIC_CUBE_MAP_H */
