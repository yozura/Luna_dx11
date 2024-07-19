#ifndef BILLBOARDS_H
#define BILLBOARDS_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class Billboards : public D3DApp
{
public:
	Billboards(HINSTANCE hInstance);
    ~Billboards();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void CreateShaderResourceViewFromFile();
	void BuildRoomGeometryBuffers();
	void BuildSkullGeometryBuffers();

private:
	ID3D11Buffer* mRoomVertexBuffer;
	
	ID3D11Buffer* mSkullVertexBuffer;
	ID3D11Buffer* mSkullIndexBuffer;
	
	ID3D11ShaderResourceView* mFloorDiffuseMapSRV;
	ID3D11ShaderResourceView* mWallDiffuseMapSRV;
	ID3D11ShaderResourceView* mMirrorDiffuseMapSRV;

	DirectionalLight mDirLights[3];
	
	Material mRoomMat;
	Material mSkullMat;
	Material mMirrorMat;
	Material mShadowMat;

	DirectX::XMFLOAT4X4 mRoomWorld;
	DirectX::XMFLOAT4X4 mSkullWorld;
	DirectX::XMFLOAT4X4 mTexTransform;

	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	DirectX::XMFLOAT3 mSkullTranslation;
	DirectX::XMFLOAT3 mEyePosW;

	RenderOptions mRenderOptions;

	UINT mSkullIndexCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif /* MIRROR_H */
