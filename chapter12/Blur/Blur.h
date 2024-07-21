#ifndef BLUR_H
#define BLUR_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "Waves.h"
#include "BlurFilter.h"

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class Blur : public D3DApp
{
public:
    Blur(HINSTANCE hInstance);
    ~Blur();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void DrawWrapper();
    void DrawScreenQuad();
    float GetHillHeight(float x, float z) const;
    DirectX::XMFLOAT3 GetHillNormal(float x, float z) const;
    void BuildLandGeometryBuffers();
    void BuildWaveGeometryBuffers();
    void BuildCrateGeometryBuffers();
    void BuildScreenQuadGeometryBuffers();
    void BuildOffscreenViews();

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;
	
	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;
	
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

    ID3D11Buffer* mScreenQuadVB;
    ID3D11Buffer* mScreenQuadIB;
	
	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWaterMapSRV;
	ID3D11ShaderResourceView* mCrateSRV;

    ID3D11ShaderResourceView*  mOffscreenSRV;
    ID3D11UnorderedAccessView* mOffscreenUAV;
    ID3D11RenderTargetView*    mOffscreenRTV;

    BlurFilter mBlur;
	Waves      mWaves;

	DirectionalLight mDirLights[3];

	Material mLandMat;
	Material mWavesMat;
	Material mBoxMat;

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
	UINT mWavesIndexCount;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif /* BLUR_H */
