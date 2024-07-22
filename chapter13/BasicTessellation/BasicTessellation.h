#ifndef BASIC_TESSELLATION_H
#define BASIC_TESSELLATION_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"

class BasicTessellation : public D3DApp
{
public:
    BasicTessellation(HINSTANCE hInstance);
    ~BasicTessellation();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
    void BuildQuadPatchBuffer();

private:
    ID3D11Buffer* mQuadPatchVB;
	
	DirectX::XMFLOAT4X4 mView;
	DirectX::XMFLOAT4X4 mProj;

	DirectX::XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif /* BASIC_TESSELLATION_H */
