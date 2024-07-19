#include "Lighthelper.fx"

cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
    
    float4 gFogColor;
    float  gFogStart;
    float  gFogRange;
};

cbuffer cbPerObject
{
    float4x4 gViewProj;
    Material gMaterial;
};

cbuffer cbFixed
{
    float2 gTexC[4] =
    {
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f)
    };
};

Texture2DArray gTreeMapArray;

SamplerState samLinear
{
    Filter   = MIN_MAG_MIP_LINEAR;

    AddressU = CLAMP;
    AddressU = CLAMP;
};

struct VertexIn
{
    float3 PosW  : POSITION;
    float2 SizeW : SIZE;
};

struct VertexOut
{
    float3 CenterW : POSITION;
    float2 SizeW   : SIZE;
};

struct GeoOut
{
    float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex     : TEXCOORD;
    uint   PrimID  : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    vout.CenterW = vin.PosW;
    vout.SizeW   = vin.SizeW;
    
    return vout;
}

[maxvertexcount(4)]
void GS(point VertexOut gin[1],
        uint primID : SV_PrimitiveID,
        inout TriangleStream<GeoOut> triStream)
{
    float3 up   = float3(0.0f, 1.0f, 0.0f);
    float3 look = gEyePosW - gin[0].CenterW;
    look.y = 0.0f;
    look = normalize(look);
    float3 right = cross(up, look);
    
    float halfWidth  = 0.5f * gin[0].SizeW.x;
    float halfHeight = 0.5f * gin[0].SizeW.y;
    
    float4 v[4];
    v[0] = float4(gin[0].CenterW + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(gin[0].CenterW + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(gin[0].CenterW - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(gin[0].CenterW - halfWidth * right + halfHeight * up, 1.0f);

    GeoOut gout;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        gout.PosH    = mul(v[i], gViewProj);
        gout.PosW    = v[i].xyz;
        gout.NormalW = look;
        gout.Tex     = gTexC[i];
        gout.PrimID  = primID;
        
        triStream.Append(gout);
    }
}

float PS(GeoOut pin, uniform int gLightCount, uniform bool gUseTexture, uniform bool gAlphaClip, uniform bool gFogEnabled) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEye = gEyePosW - pin.PosW;
    
    float distToEye = length(toEye);

    toEye /= distToEye;
    
    float4 texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    if (gUseTexture)
    {
        float3 uvw = float3(pin.Tex, pin.PrimID % 4);
        texColor = gTreeMapArray.Sample(samLinear, uvw);

        if (gAlphaClip)
        {
            clip(texColor.a - 0.05f);
        }
    }
    
    float4 litColor = texColor;
    
    if (gLightCount > 0)
    {
        float4 ambient  = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

        [unroll]
        for (int i = 0; i < gLightCount; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);
            ambient  += A;
            diffuse  += D;
            specular += S;
        }
        
        litColor = texColor * (ambient + diffuse) + specular;
    }
    
    if (gFogEnabled)
    {
        float fogLerp = saturate((distToEye - gFogRange) / gFogRange);
        litColor = lerp(litColor, gFogColor, fogLerp);
    }
    
    litColor.a = gMaterial.Diffuse.a * texColor.a;
    
    return litColor;
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false)));
    }
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, false)));
    }
}
            
technique11 Light3TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, true)));
    }
}