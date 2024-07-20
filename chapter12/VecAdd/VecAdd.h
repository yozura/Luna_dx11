#ifndef VECADD_H
#define VECADD_H

#include "D3DApp.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Waves.h"
#include "Vertex.h"
#include "Effects.h"
#include "RenderStates.h"

struct Data
{
    DirectX::XMFLOAT3 v1;
    DirectX::XMFLOAT2 v2;
};

class VecAdd : public D3DApp
{
public:
    VecAdd(HINSTANCE hInstance);
	~VecAdd();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

    void DoComputeWork();
private:
    void BuildBuffersAndView();

private:
    ID3D11Buffer* mOutputBuffer;
    ID3D11Buffer* mOutputDebugBuffer;

    ID3D11ShaderResourceView*  mInputASRV;
    ID3D11ShaderResourceView*  mInputBSRV;
    ID3D11UnorderedAccessView* mOutputUAV;

    UINT mNumElements;
};

#endif /* VECADD_H */
