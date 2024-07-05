#ifndef SHAPES_H
#define SHAPES_H

#include "D3DApp.h"
#include "GeometryGenerator.h"

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

class Shapes : public D3DApp
{
public:
    Shapes(HINSTANCE hInstance);
    ~Shapes();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;
	
	ID3D11RasterizerState* mWireFrameRS;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylinderWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

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
	
	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

#endif /* SHAPES_H */

