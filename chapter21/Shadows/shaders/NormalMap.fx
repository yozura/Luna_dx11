#include "LightHelper.fx"

cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
    
    float  gFogStart;
    float  gFogRange;
    float4 gFogColor;
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
    float4x4 gShadowTrasnform;
    Material gMaterial;
};

Texture2D gShadowMap;
Texture2D gDiffuseMap;
Texture2D gNormalMap;
TextureCube gCubeMap;

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;

    AddressU = WRAP;
    AddressV = WRAP;
};

SamplerComparisonState samShadow
{
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    AddressW = BORDER;
    BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ComparisonFunc = LESS_EQUAL;
};

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float2 Tex      : TEXCOORD;
    float3 TangentL : TANGENT;
};

struct VertexOut
{
    float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float2 Tex        : TEXCOORD0;
    float4 ShadowPosH : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Transform to world space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
    vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);
    
    // Transform to homogeneous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
    
    // Output vertex attribute for interpolation across triangle.
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
    
    vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTrasnform);
    
    return vout;
}

float4 PS(VertexOut pin,
          uniform int gLightCount,
          uniform bool gUseTexture,
          uniform bool gAlphaClip,
          uniform bool gFogEnabled,
          uniform bool gReflectionEnabled) : SV_Target
{
    pin.NormalW = normalize(pin.NormalW);
    
    float3 toEye = gEyePosW - pin.PosW;
    float distToEye = length(toEye);
    toEye /= distToEye;

    float4 texColor = float4(1, 1, 1, 1);
    if (gUseTexture)
    {
        texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
        
        if (gAlphaClip)
        {
            clip(texColor.a - 0.1f);
        }
    }

    // Normal mapping
    float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
    
    // Lighting
    float4 litColor = texColor;
    if (gLightCount > 0)
    {
        float4 ambient  = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
        
        float3 shadow = float3(1.0f, 1.0f, 1.0f);
        shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);
        
        [unroll]
        for (int i = 0; i < gLightCount; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], bumpedNormalW, toEye, A, D, S);
            
            ambient  += A;
            diffuse  += shadow[i] * D;
            specular += shadow[i] * S;
        }
        
        litColor = texColor * (ambient + diffuse) + specular;
        
        if (gReflectionEnabled)
        {
            float3 incident = -toEye;
            float3 reflectionVector = reflect(incident, bumpedNormalW);
            float4 reflectionColor = gCubeMap.Sample(samLinear, reflectionVector);
            
            litColor += gMaterial.Reflect * reflectionColor;
        }
    }
    
	// Fogging
    if (gFogEnabled)
    {
        float fogLerp = saturate((distToEye - gFogStart) / gFogRange);
        litColor = lerp(litColor, gFogColor, fogLerp);
    }

    litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 Light1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, false, false)));
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, false, false)));
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false, false)));
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, false, false)));
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, false, false)));
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, false, false)));
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, false, false)));
    }
}

technique11 Light0TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, false, false)));
    }
}

technique11 Light1TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, false, false)));
    }
}

technique11 Light2TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, false, false)));
    }
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, false, false)));
    }
}

technique11 Light1Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, true, false)));
    }
}

technique11 Light2Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, true, false)));
    }
}

technique11 Light3Fog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, true, false)));
    }
}

technique11 Light0TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, true, false)));
    }
}

technique11 Light1TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, true, false)));
    }
}

technique11 Light2TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, true, false)));
    }
}

technique11 Light3TexFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, true, false)));
    }
}

technique11 Light0TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, true, false)));
    }
}

technique11 Light1TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, true, false)));
    }
}

technique11 Light2TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, true, false)));
    }
}

technique11 Light3TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, true, false)));
    }
}

technique11 Light1Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, false, true)));
    }
}

technique11 Light2Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, false, true)));
    }
}

technique11 Light3Reflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, false, true)));
    }
}

technique11 Light0TexReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, false, true)));
    }
}

technique11 Light1TexReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, false, true)));
    }
}

technique11 Light2TexReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, false, true)));
    }
}

technique11 Light3TexReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, false, true)));
    }
}

technique11 Light0TexAlphaClipReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, false, true)));
    }
}

technique11 Light1TexAlphaClipReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, false, true)));
    }
}

technique11 Light2TexAlphaClipReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, false, true)));
    }
}

technique11 Light3TexAlphaClipReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, false, true)));
    }
}

technique11 Light1FogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false, true, true)));
    }
}

technique11 Light2FogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false, true, true)));
    }
}

technique11 Light3FogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false, true, true)));
    }
}

technique11 Light0TexFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false, true, true)));
    }
}

technique11 Light1TexFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false, true, true)));
    }
}

technique11 Light2TexFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false, true, true)));
    }
}

technique11 Light3TexFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false, true, true)));
    }
}

technique11 Light0TexAlphaClipFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, true, true, true)));
    }
}

technique11 Light1TexAlphaClipFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true, true, true)));
    }
}

technique11 Light2TexAlphaClipFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true, true, true)));
    }
}

technique11 Light3TexAlphaClipFogReflect
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true, true, true)));
    }
}