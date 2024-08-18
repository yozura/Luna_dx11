#include "TextureMgr.h"

using namespace DirectX;

TextureMgr::TextureMgr() : md3dDevice(0)
{
}

TextureMgr::~TextureMgr()
{
    for (auto it = mTextureSRV.begin(); it != mTextureSRV.end(); ++it)
    {
        ReleaseCOM(it->second);
    }

    mTextureSRV.clear();
}

void TextureMgr::Init(ID3D11Device* device)
{
    md3dDevice = device;
}

ID3D11ShaderResourceView* TextureMgr::CreateTexture(std::wstring filename)
{
    ID3D11ShaderResourceView* srv = 0;

    // Does it already exist?
    if (mTextureSRV.find(filename) != mTextureSRV.end())
    {
        srv = mTextureSRV[filename];
    }
    else
    {
        ScratchImage texture;
        HR(LoadFromDDSFile(filename.c_str(), DDS_FLAGS_NONE, nullptr, texture));
        HR(CreateShaderResourceView(md3dDevice, texture.GetImages(), texture.GetImageCount(), texture.GetMetadata(), &srv));

        mTextureSRV[filename] = srv;
    }

    return srv;
}

