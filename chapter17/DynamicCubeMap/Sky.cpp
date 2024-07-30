#include "Sky.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertex.h"
#include "Effects.h"

using namespace DirectX;

Sky::Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius)
{
    ScratchImage cubemap;
    HR(LoadFromDDSFile(cubemapFilename.c_str(), DDS_FLAGS_NONE, nullptr, cubemap));
    HR(CreateShaderResourceView(device, cubemap.GetImages(), cubemap.GetImageCount(), cubemap.GetMetadata(), &mCubeMapSRV));

    GeometryGenerator::MeshData sphere;
    GeometryGenerator geoGen;
    geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

    std::vector<XMFLOAT3> vertices(sphere.Vertices.size());
    for (size_t i = 0; i < sphere.Vertices.size(); ++i)
    {
        vertices[i] = sphere.Vertices[i].Position;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vInitData;
    vInitData.pSysMem = &vertices[0];
    
    HR(device->CreateBuffer(&vbd, &vInitData, &mVB));

    mIndexCount = sphere.Indices.size();

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(USHORT) * mIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;

    std::vector<USHORT> indices16;
    indices16.assign(sphere.Indices.begin(), sphere.Indices.end());

    D3D11_SUBRESOURCE_DATA iInitData;
    iInitData.pSysMem = &indices16[0];

    HR(device->CreateBuffer(&ibd, &iInitData, &mIB));
}

Sky::~Sky()
{
    ReleaseCOM(mVB);
    ReleaseCOM(mIB);
    ReleaseCOM(mCubeMapSRV);
}

ID3D11ShaderResourceView* Sky::CubeMapSRV()
{
    return mCubeMapSRV;
}

void Sky::Draw(ID3D11DeviceContext* dc, const Camera& camera)
{
    // Center Sky about eye in world space.
    XMFLOAT3 eyePos = camera.GetPosition();
    XMMATRIX world = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
    XMMATRIX worldViewProj = XMMatrixMultiply(world, camera.ViewProj());
    
    Effects::SkyFX->SetWorldViewProj(worldViewProj);
    Effects::SkyFX->SetCubeMap(mCubeMapSRV);

    UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;

    dc->IASetInputLayout(InputLayouts::Pos);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
    dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R16_UINT, 0);

    D3DX11_TECHNIQUE_DESC techDesc;
    Effects::SkyFX->SkyTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        Effects::SkyFX->SkyTech->GetPassByIndex(p)->Apply(0, dc);
        dc->DrawIndexed(mIndexCount, 0, 0);
    }
}
