#ifndef NORMAL_DISPLACEMENT_MAP
#define NORMAL_DISPLACEMENT_MAP

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "Sky.h"
#include "RenderStates.h"

enum RenderOptions
{
    RenderOptionsBasic = 0,
    RenderOptionsNormalMap = 1,
    RenderOptionsDisplacementMap = 2
};

class NormalDisplacementMap : public D3DApp
{
public:
    NormalDisplacementMap(HINSTANCE hInstance);
    ~NormalDisplacementMap();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();

private:
    Sky* mSky;

	ID3D11Buffer* mShapesVB;
	ID3D11Buffer* mShapesIB;

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;

    ID3D11Buffer* mSkySphereVB;
    ID3D11Buffer* mSkySphereIB;

    ID3D11ShaderResourceView* mStoneTexSRV;
    ID3D11ShaderResourceView* mBrickTexSRV;

    ID3D11ShaderResourceView* mStoneNormalTexSRV;
    ID3D11ShaderResourceView* mBrickNormalTexSRV;
	
	DirectionalLight mDirLights[3];
	Material mGridMat;
	Material mBoxMat;
	Material mCylinderMat;
	Material mSphereMat;
	Material mSkullMat;

	DirectX::XMFLOAT4X4 mSphereWorld[10];
	DirectX::XMFLOAT4X4 mCylinderWorld[10];
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

	UINT mLightCount;

    RenderOptions mRenderOptions;

    Camera mCam;

	POINT mLastMousePos;
};

#endif /* NORMAL_DISPLACEMENT_MAP */
