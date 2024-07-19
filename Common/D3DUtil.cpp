#include "D3DUtil.h"

using namespace DirectX;

ID3D11ShaderResourceView* D3DHelper::CreateTexture2DArraySRV(ID3D11Device* device,
                                                             ID3D11DeviceContext* context,
                                                             std::vector<std::wstring>& filenames,
                                                             DXGI_FORMAT format)
{
    UINT size = filenames.size();
    std::vector<ID3D11Texture2D*> srcTex(size);
    for (UINT i = 0; i < size; ++i)
    {
        ScratchImage texture;
        ScratchImage convertedTexture;
        ScratchImage mipChain;

        HR(LoadFromDDSFile(filenames[i].c_str(), DDS_FLAGS_NONE, nullptr, texture));
        if (texture.GetMetadata().format == DXGI_FORMAT_BC1_UNORM)
        {
            HR(Decompress(texture.GetImages(), texture.GetImageCount(), texture.GetMetadata(), format, convertedTexture));
        }
        else
        {
            HR(Convert(texture.GetImages(), texture.GetImageCount(), texture.GetMetadata(), format, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, convertedTexture));
        }
        
        HR(GenerateMipMaps(convertedTexture.GetImages(), convertedTexture.GetImageCount(), convertedTexture.GetMetadata(), TEX_FILTER_LINEAR, 0, mipChain));

        HR(CreateTextureEx(device, 
                           mipChain.GetImages(),
                           mipChain.GetImageCount(),
                           mipChain.GetMetadata(),
                           D3D11_USAGE_STAGING,
                           0,
                           D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ,
                           0, CREATETEX_DEFAULT,
                           reinterpret_cast<ID3D11Resource**>(&srcTex[i])));
    }

    // Create the texture array description
    D3D11_TEXTURE2D_DESC texElementDesc;
    srcTex[0]->GetDesc(&texElementDesc);

    D3D11_TEXTURE2D_DESC texArrayDesc;
    texArrayDesc.Width              = texElementDesc.Width;
    texArrayDesc.Height             = texElementDesc.Height;
    texArrayDesc.MipLevels          = texElementDesc.MipLevels;
    texArrayDesc.ArraySize          = size;
    texArrayDesc.Format             = texElementDesc.Format;
    texArrayDesc.SampleDesc.Count   = 1;
    texArrayDesc.SampleDesc.Quality = 0;
    texArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
    texArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
    texArrayDesc.CPUAccessFlags     = 0;
    texArrayDesc.MiscFlags          = 0;

    ID3D11Texture2D* texArray = 0;
    HR(device->CreateTexture2D(&texArrayDesc, 0, &texArray));
    
    // Copy individual texture elements into the texture array
    for (UINT texElement = 0; texElement < size; ++texElement)
    {
        for (UINT mipLevel = 0; mipLevel < texArrayDesc.MipLevels; ++mipLevel)
        {
            D3D11_MAPPED_SUBRESOURCE mappedTex2D;
            HR(context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

            context->UpdateSubresource(texArray,
                                       D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
                                       0,
                                       mappedTex2D.pData,
                                       mappedTex2D.RowPitch,
                                       mappedTex2D.DepthPitch);

            context->Unmap(srcTex[texElement], mipLevel);
        }
    }

    // Create a shader resource view to the texture array
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
    viewDesc.Format = texElementDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MostDetailedMip = 0;
    viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
    viewDesc.Texture2DArray.FirstArraySlice = 0;
    viewDesc.Texture2DArray.ArraySize = size;

    ID3D11ShaderResourceView* texArraySRV = 0;
    HR(device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

    // Cleanup
    ReleaseCOM(texArray);
    for (UINT i = 0; i < size; ++i)
        ReleaseCOM(srcTex[i]);

    return texArraySRV;
}
