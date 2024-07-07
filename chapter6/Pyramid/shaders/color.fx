cbuffer cbPerObject
{
    float4x4 gWorldViewProj;
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
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}