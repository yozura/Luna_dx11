cbuffer cbPerObject
{
    float4x4 gWorldViewProj;
    float    gTime;
};

RasterizerState WireFrame
{
    CullMode = BACK;
    FillMode = WIREFRAME;
};

RasterizerState Solid
{
    CullMode = BACK;
    FillMode = SOLID;
};

struct VertexIn
{
    float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertextOut
{
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertextOut VS(VertexIn vin)
{
    VertextOut vout;
    
    vin.PosL.xy += 0.5f * sin(vin.PosL.x) * sin(3.0f * gTime);
    vin.PosL.z  *= 0.6f + 0.4f * sin(2.0f * gTime);
    
    // 동차 변환 3D => 4D
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
   
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertextOut pin) : SV_Target
{
    return pin.Color;
}

technique11 ColorTech
{
    pass P0
    {
        SetRasterizerState(Solid);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}