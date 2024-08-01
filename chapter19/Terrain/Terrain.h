#ifndef TERRAIN_H
#define TERRAIN_H

#include "D3DUtil.h"
#include "LightHelper.h"

class Camera;

class Terrain
{
public:
    struct InitInfo
    {
        std::wstring HeightMapFilename;
        std::wstring LayerMapFilename0;
        std::wstring LayerMapFilename1;
        std::wstring LayerMapFilename2;
        std::wstring LayerMapFilename3;
        std::wstring LayerMapFilename4;
        std::wstring BlendMapFilename;

        float HeightScale;
        UINT HeightMapWidth;
        UINT HeightMapHeight;
        float CellSpacing;
    };

public:
    Terrain();
    ~Terrain();

    float GetWidth() const;
    float GetDepth() const;
    float GetHeight(float x, float z) const;

    DirectX::XMMATRIX GetWorld() const;
    void SetWorld(DirectX::CXMMATRIX M);

    void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);

    void Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3]);

private:
    void LoadHeightmap();
    void Smooth();
    bool InBounds(int i, int j);
    float Average(int i, int j);
    void CalcAllPatchBoundsY();
    void CalcPatchBoundsY(UINT i, UINT j);
    void BuildQuadPatchVB(ID3D11Device* device);
    void BuildQuadPatchIB(ID3D11Device* device);
    void BuildHeightmapSRV(ID3D11Device* device);

private:
    static const int CellsPerPatch = 64;

    ID3D11Buffer* mQuadPatchVB;
    ID3D11Buffer* mQuadPatchIB;

    ID3D11ShaderResourceView* mLayerMapArraySRV;
    ID3D11ShaderResourceView* mBlendMapSRV;
    ID3D11ShaderResourceView* mHeightMapSRV;

    InitInfo mInfo;

    UINT mNumPatchVertices;
    UINT mNumPatchQuadFaces;

    UINT mNumPatchVertRows;
    UINT mNumPatchVertCols;

    DirectX::XMFLOAT4X4 mWorld;

    Material mMat;

    std::vector<DirectX::XMFLOAT2> mPatchBoundsY;
    std::vector<float> mHeightMap;
};

#endif /* TERRAIN_H */