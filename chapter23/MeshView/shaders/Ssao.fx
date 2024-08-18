cbuffer cbPerFrame
{
    float4x4 gViewToTexSpace; // Proj * Texture
    float4 gOffsetVectors[14];
    float4 gFrustumCorners[4];

    // Coordinates given in view space.
    float gOcclusionRadius = 0.5f;
    float gOcclusionFadeStart = 0.2f;
    float gOcclusionFadeEnd = 2.0f;
    float gSurfaceEpsilon = 0.05f;
};

Texture2D gNormalDepthMap;
Texture2D gRandomVecMap;

SamplerState samNormalDepth
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = float4(0.0f, 0.0f, 0.0f, 1e5f);
};

SamplerState samRandomVec
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
};

struct VertexIn
{
    float3 PosL            : POSITION;
    float3 ToFarPlaneIndex : NORMAL;
    float2 Tex             : TEXCOORD;
};

struct VertexOut
{
    float4 PosH       : SV_Position;
    float3 ToFarPlane : TEXCOORD0;
    float2 Tex        : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosH = float4(vin.PosL, 1.0f);

    vout.ToFarPlane = gFrustumCorners[vin.ToFarPlaneIndex.x].xyz;

    vout.Tex = vin.Tex;

    return vout;
}

float OcclusionFunction(float distZ)
{
    float occlusion = 0.0f;
    if (distZ > gSurfaceEpsilon)
    {
        float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
        occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
    }
    return occlusion;
}

float4 PS(VertexOut pin, uniform int gSampleCount) : SV_Target
{
    float4 normalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
    float3 n = normalDepth.xyz;
    float pz = normalDepth.w;

    float3 p = (pz / pin.ToFarPlane.z) * pin.ToFarPlane;

    float3 randVec = 2.0f * gRandomVecMap.SampleLevel(samRandomVec, 4.0f * pin.Tex, 0.0f).rgb - 1.0f;

    float occlusionSum = 0.0f;

    [unroll]
    for (int i = 0; i < gSampleCount; ++i)
    {
        float3 offset = reflect(gOffsetVectors[i].xyz, randVec);

        float flip = sign(dot(offset, n));

        float3 q = p + flip * gOcclusionRadius * offset;

        float4 projQ = mul(float4(q, 1.0f), gViewToTexSpace);
        projQ /= projQ.w;

        float rz = gNormalDepthMap.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;

        float3 r = (rz / q.z) * q;

        float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);
        float occlusion = dp * OcclusionFunction(distZ);

        occlusionSum += occlusion;
    }

    occlusionSum /= gSampleCount;

    float access = 1.0f - occlusionSum;

    return saturate(pow(access, 4.0f));
}

technique11 Ssao
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(14)));
    }
}
