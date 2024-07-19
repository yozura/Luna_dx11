#ifndef BILLBOARDS_H
#define BILLBOARDS_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "Waves.h"

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
	float GetHillHeight(float x, float z) const;
	DirectX::XMFLOAT3 GetHillNormal(float x, float z) const;
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildCrateGeometryBuffers();
	void BuildTreeSpritesBuffer();
	void DrawTreeSprites(DirectX::CXMMATRIX viewProj);

private:
	ID3D11Buffer* mLandVertexBuffer;
	ID3D11Buffer* mLandIndexBuffer;
	
	ID3D11Buffer* mWavesVertexBuffer;
	ID3D11Buffer* mWavesIndexBuffer;
	
	ID3D11Buffer* mBoxVertexBuffer;
	ID3D11Buffer* mBoxIndexBuffer;

	ID3D11Buffer* mTreeSpriteVertexBuffer;
	
	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWaterMapSRV;
	ID3D11ShaderResourceView* mBoxMapSRV;
	ID3D11ShaderResourceView* mTreeTextureMapArraySRV;

	Waves mWaves;

	DirectionalLight mDirLights[3];
	
	Material mLandMat;
	Material mWavesMat;
	Material mBoxMat;
	Material mTreeMat;

	DirectX::XMFLOAT4X4 mGrassTexTransform;
	DirectX::XMFLOAT4X4 mWaterTexTransform;
	DirectX::XMFLOAT4X4 mLandWorld;
	DirectX::XMFLOAT4X4 mWavesWorld;
	DirectX::XMFLOAT4X4 mBoxWorld;

	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	DirectX::XMFLOAT3 mEyePosW;

	DirectX::XMFLOAT2 mWaterTexOffset;

	RenderOptions mRenderOptions;

	UINT mLandIndexCount;

	static const UINT TreeCount = 16;

	bool mAlphaToCoverageOn;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif /* MIRROR_H */
