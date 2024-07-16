#ifndef BLEND_H
#define BLEND_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Vertex.h"
#include "Effects.h"
#include "Waves.h"

class Blend : public D3DApp
{
public:
	Blend(HINSTANCE hInstance);
	~Blend();

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

private:
	ID3D11Buffer* mLandVertexBuffer;
	ID3D11Buffer* mLandIndexBuffer;
	
	ID3D11Buffer* mWavesVertexBuffer;
	ID3D11Buffer* mWavesIndexBuffer;

	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWaterMapSRV;

	Waves mWaves;

	DirectionalLight mDirLights[3];
	Material mLandMat;
	Material mWavesMat;

	DirectX::XMFLOAT4X4 mGrassTexTransform;
	DirectX::XMFLOAT4X4 mWaterTexTransform;
	DirectX::XMFLOAT4X4 mLandWorld;
	DirectX::XMFLOAT4X4 mWavesWorld;

	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	UINT mLandIndexCount;
	DirectX::XMFLOAT2 mWaterTexOffset;

	DirectX::XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif /* BLEND_H */
