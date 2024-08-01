#include "Terrain.h"
#include "Camera.h"
#include "Effects.h"
#include "Vertex.h"
#include "MathHelper.h"

using namespace DirectX;

Terrain::Terrain()
    : mQuadPatchVB(0), mQuadPatchIB(0)
    , mLayerMapArraySRV(0), mBlendMapSRV(0), mHeightMapSRV(0)
    , mNumPatchVertices(0), mNumPatchQuadFaces(0)
    , mNumPatchVertRows(0), mNumPatchVertCols(0)
{
    XMStoreFloat4x4(&mWorld, XMMatrixIdentity());

    mMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
    mMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

Terrain::~Terrain()
{
    ReleaseCOM(mQuadPatchVB);
    ReleaseCOM(mQuadPatchIB);
    ReleaseCOM(mLayerMapArraySRV);
    ReleaseCOM(mBlendMapSRV);
    ReleaseCOM(mHeightMapSRV);
}

float Terrain::GetWidth() const
{
    return (mInfo.HeightMapWidth - 1) * mInfo.CellSpacing;
}

float Terrain::GetDepth() const
{
    return (mInfo.HeightMapHeight - 1) * mInfo.CellSpacing;
}

float Terrain::GetHeight(float x, float z) const
{
    // Transform from terrain local space to "cell" space.
    float c = (x + 0.5f * GetWidth()) / mInfo.CellSpacing;
    float d = (z - 0.5f * GetDepth()) / -mInfo.CellSpacing;

    int row = (int)floorf(d);
    int col = (int)floorf(c);

    // Grab the heights of the cell we are in.
    // A*--*B
    //  | /|
    //  |/ |
    // C*--*D
    float A = mHeightMap[row * mInfo.HeightMapWidth + col];
    float B = mHeightMap[row * mInfo.HeightMapWidth + col + 1];
    float C = mHeightMap[(row + 1) * mInfo.HeightMapWidth + col];
    float D = mHeightMap[(row + 1) * mInfo.HeightMapWidth + col + 1];

    float s = c - (float)col;
    float t = d - (float)row;

    if (s + t <= 1.0f)
    {
        float uy = B - A;
        float vy = C - A;
        return A + s * uy + t * vy;
    }
    else
    {
        float uy = C - D;
        float vy = B - D;
        return D + (1.0f - s) * uy + (1.0f - t) * vy;
    }
}

XMMATRIX Terrain::GetWorld() const
{
    return XMLoadFloat4x4(&mWorld);
}

void Terrain::SetWorld(CXMMATRIX M)
{
    XMStoreFloat4x4(&mWorld, M);
}

void Terrain::Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo)
{
    mInfo = initInfo;

    // Divide heightmap into patches such that each patch has CellsPerPatch.
    mNumPatchVertRows = ((mInfo.HeightMapHeight - 1) / CellsPerPatch) + 1;
    mNumPatchVertCols = ((mInfo.HeightMapWidth - 1) / CellsPerPatch) + 1;

    mNumPatchVertices  = mNumPatchVertRows * mNumPatchVertCols;
    mNumPatchQuadFaces = (mNumPatchVertRows - 1) * (mNumPatchVertCols - 1);

    LoadHeightmap();
    Smooth();
    CalcAllPatchBoundsY();
    
    BuildQuadPatchVB(device);
    BuildQuadPatchIB(device);
    BuildHeightmapSRV(device);

    std::vector<std::wstring> layerFilenames;
    layerFilenames.push_back(mInfo.LayerMapFilename0);
    layerFilenames.push_back(mInfo.LayerMapFilename1);
    layerFilenames.push_back(mInfo.LayerMapFilename2);
    layerFilenames.push_back(mInfo.LayerMapFilename3);
    layerFilenames.push_back(mInfo.LayerMapFilename4);
    mLayerMapArraySRV = D3DHelper::CreateTexture2DArraySRV(device, dc, layerFilenames, DXGI_FORMAT_R8G8B8A8_UNORM);

    ScratchImage blend;
    HR(LoadFromDDSFile(mInfo.BlendMapFilename.c_str(), DDS_FLAGS_NONE, nullptr, blend));
    HR(CreateShaderResourceView(device, blend.GetImages(), blend.GetImageCount(), blend.GetMetadata(), &mBlendMapSRV));
}

void Terrain::Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3])
{
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
    dc->IASetInputLayout(InputLayouts::Terrain);

    UINT stride = sizeof(Vertex::Terrain);
    UINT offset = 0;

    dc->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
    dc->IASetIndexBuffer(mQuadPatchIB, DXGI_FORMAT_R16_UINT, 0);

    XMMATRIX viewProj = cam.ViewProj();
    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
    XMMATRIX worldViewProj = world * viewProj;

    XMFLOAT4 worldPlanes[6];
    ExtractFrustumPlanes(worldPlanes, viewProj);

    Effects::TerrainFX->SetViewProj(viewProj);
    Effects::TerrainFX->SetEyePosW(cam.GetPosition());
    Effects::TerrainFX->SetDirLights(lights);
    Effects::TerrainFX->SetFogColor(Colors::Silver);
    Effects::TerrainFX->SetFogStart(15.0f);
    Effects::TerrainFX->SetFogRange(175.0f);
    Effects::TerrainFX->SetMinDist(20.0f);
    Effects::TerrainFX->SetMaxDist(500.0f);
    Effects::TerrainFX->SetMinTess(0.0f);
    Effects::TerrainFX->SetMaxTess(6.0f);
    Effects::TerrainFX->SetTexelCellSpaceU(1.0f / mInfo.HeightMapWidth);
    Effects::TerrainFX->SetTexelCellSpaceV(1.0f / mInfo.HeightMapHeight);
    Effects::TerrainFX->SetWorldCellSpace(mInfo.CellSpacing);
    Effects::TerrainFX->SetWorldFrustumPlanes(worldPlanes);

    Effects::TerrainFX->SetLayerMapArray(mLayerMapArraySRV);
    Effects::TerrainFX->SetBlendMap(mBlendMapSRV);
    Effects::TerrainFX->SetHeightMap(mHeightMapSRV);

    Effects::TerrainFX->SetMaterial(mMat);

    ID3DX11EffectTechnique* tech = Effects::TerrainFX->Light1Tech;
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        tech->GetPassByIndex(p)->Apply(0, dc);
        dc->DrawIndexed(mNumPatchQuadFaces * 4, 0, 0);
    }

    dc->HSSetShader(0, 0, 0);
    dc->DSSetShader(0, 0, 0);
}

void Terrain::LoadHeightmap()
{
    std::vector<unsigned char> in(mInfo.HeightMapWidth * mInfo.HeightMapHeight);

    std::ifstream inFile;
    inFile.open(mInfo.HeightMapFilename.c_str(), std::ios_base::binary);
    if (inFile)
    {
        inFile.read((char*)&in[0], (std::streamsize)in.size());
        inFile.close();
    }

    mHeightMap.resize(mInfo.HeightMapHeight * mInfo.HeightMapWidth, 0);
    for (UINT i = 0; i < mInfo.HeightMapHeight * mInfo.HeightMapWidth; ++i)
    {
        mHeightMap[i] = (in[i] / 255.0f) * mInfo.HeightScale;
    }
}

void Terrain::Smooth()
{
    std::vector<float> dest(mHeightMap.size());
    for (UINT i = 0; i < mInfo.HeightMapHeight; ++i)
    {
        for (UINT j = 0; j < mInfo.HeightMapWidth; ++j)
        {
            dest[i * mInfo.HeightMapWidth + j] = Average(i, j);
        }
    }

    mHeightMap = dest;
}

bool Terrain::InBounds(int i, int j)
{
    return i >= 0 && i < (int)mInfo.HeightMapHeight &&
           j >= 0 && j < (int)mInfo.HeightMapWidth;
}

float Terrain::Average(int i, int j)
{
    float avg = 0.0f;
    float num = 0.0f;

    for (int m = i - 1; m <= i + 1; ++m)
    {
        for (int n = j - 1; n <= j + 1; ++n)
        {
            if (InBounds(m, n))
            {
                avg += mHeightMap[m * mInfo.HeightMapWidth + n];
                num += 1.0f;
            }
        }
    }

    return avg / num;
}

void Terrain::CalcAllPatchBoundsY()
{
    mPatchBoundsY.resize(mNumPatchQuadFaces);

    for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
    {
        for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
        {
            CalcPatchBoundsY(i, j);
        }
    }
}

void Terrain::CalcPatchBoundsY(UINT i, UINT j)
{
    UINT x0 = j * CellsPerPatch;
    UINT x1 = (j + 1) * CellsPerPatch;

    UINT y0 = i * CellsPerPatch;
    UINT y1 = (i + 1) * CellsPerPatch;

    float minY = +MathHelper::Infinity;
    float maxY = -MathHelper::Infinity;

    for (UINT y = y0; y <= y1; ++y)
    {
        for (UINT x = x0; x <= x1; ++x)
        {
            UINT k = y * mInfo.HeightMapWidth + x;
            minY = MathHelper::Min(minY, mHeightMap[k]);
            maxY = MathHelper::Max(maxY, mHeightMap[k]);
        }
    }

    UINT patchID = i * (mNumPatchVertCols - 1) + j;
    mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

void Terrain::BuildQuadPatchVB(ID3D11Device* device)
{
    std::vector<Vertex::Terrain> patchVertices(mNumPatchVertRows * mNumPatchVertCols);

    float halfWidth = 0.5f * GetWidth();
    float halfDepth = 0.5f * GetDepth();

    float patchWidth = GetWidth() / (mNumPatchVertCols - 1);
    float patchDepth = GetDepth() / (mNumPatchVertRows - 1);
    float du = 1.0f / (mNumPatchVertCols - 1);
    float dv = 1.0f / (mNumPatchVertRows - 1);

    for (UINT i = 0; i < mNumPatchVertRows; ++i)
    {
        float z = halfDepth - i * patchDepth;
        for (UINT j = 0; j < mNumPatchVertCols; ++j)
        {
            float x = -halfWidth + j * patchWidth;

            patchVertices[i * mNumPatchVertCols + j].Pos = XMFLOAT3(x, 0.0f, z);

            // Stretch texture over grid.
            patchVertices[i * mNumPatchVertCols + j].Tex.x = j * du;
            patchVertices[i * mNumPatchVertCols + j].Tex.y = i * dv;
        }
    }

    // Store axis-aligned bounding box y-bounds in upper-left patch corner.
    for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
    {
        for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
        {
            UINT patchID = i * (mNumPatchVertCols - 1) + j;
            patchVertices[i * mNumPatchVertCols + j].BoundsY = mPatchBoundsY[patchID];
        }
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Terrain) * patchVertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &patchVertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB));
}

void Terrain::BuildQuadPatchIB(ID3D11Device* device)
{
    std::vector<USHORT> indices(mNumPatchQuadFaces * 4); // 4 indices per quad face

    // Iterate over each quad and compute indices.
    int k = 0;
    for (UINT i = 0; i < mNumPatchVertRows - 1; ++i)
    {
        for (UINT j = 0; j < mNumPatchVertCols - 1; ++j)
        {
            // Top row of 2x2 quad patch
            indices[k] = i * mNumPatchVertCols + j;
            indices[k + 1] = i * mNumPatchVertCols + j + 1;

            // Bottom row of 2x2 quad patch
            indices[k + 2] = (i + 1) * mNumPatchVertCols + j;
            indices[k + 3] = (i + 1) * mNumPatchVertCols + j + 1;

            k += 4; // next quad
        }
    }

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(USHORT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(device->CreateBuffer(&ibd, &iinitData, &mQuadPatchIB));
}

void Terrain::BuildHeightmapSRV(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = mInfo.HeightMapWidth;
    texDesc.Height = mInfo.HeightMapHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    std::vector<PackedVector::HALF> hmap(mHeightMap.size());
    std::transform(mHeightMap.begin(), mHeightMap.end(), hmap.begin(), PackedVector::XMConvertFloatToHalf);

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &hmap[0];
    data.SysMemPitch = mInfo.HeightMapWidth * sizeof(PackedVector::HALF);
    data.SysMemSlicePitch = 0;

    ID3D11Texture2D* hmapTex = 0;
    HR(device->CreateTexture2D(&texDesc, &data, &hmapTex));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;
    HR(device->CreateShaderResourceView(hmapTex, &srvDesc, &mHeightMapSRV));

    ReleaseCOM(hmapTex);
}