#ifndef CRATE_H
#define CRATE_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Vertex.h"
#include "Effects.h"

class TexturedHillsAndWaves : public D3DApp
{
public:
	TexturedHillsAndWaves(HINSTANCE hInstance);
	~TexturedHillsAndWaves();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();

private:
	ID3D11Buffer* mBoxVertexBuffer;
	ID3D11Buffer* mBoxIndexBuffer;

	ID3D11ShaderResourceView* mDiffuseMapSRV;

	DirectionalLight mDirLights[3];
	Material mBoxMat;

	DirectX::XMFLOAT4X4 mTexTransform;
	DirectX::XMFLOAT4X4 mBoxWorld;
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;

	DirectX::XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif /* CRATE_H */
