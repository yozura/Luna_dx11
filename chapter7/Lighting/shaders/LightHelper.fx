struct DirectionalLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 Direction;
    float  pad;
};

struct PointLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    
    float3 Position;
    float  Range;
    
    float3 Att;
    float  pad;
};

struct SpotLight
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
   
    float3 Position;
    float  Range;
    
    float3 Direction;
    float  Spot;
    
    float3 Att;
    float  pad;
};

struct Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular; // w = specPower;
    float4 Reflect;
};

float4 ToonShadingForDiffuse(float4 color, float intensity)
{
    if (intensity > 0.5f && intensity <= 1.0f)
    {
        return color * 1.0f;
    }
    else if (intensity > 0.0f && intensity <= 0.5f)
    {
        return color * 0.6f;
    }
    else
    {
        return color * 0.4f;
    }
}

float4 ToonShadingForSpecular(float4 color, float intensity)
{
    if (intensity > 0.8f && intensity <= 1.0f)
    {
        return color * 0.8f;
    }
    else if (intensity > 0.1f && intensity <= 0.8f)
    {
        return color * 0.5f;
    }
    else
    {
        return color * 0.0f;
    }
}

void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye,
                             out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightVec = -L.Direction;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v              = reflect(-lightVec, normal);
        float  specularFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        
        // diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
        // specular = specularFactor * mat.Specular * L.Specular;
        diffuse = ToonShadingForDiffuse(mat.Diffuse * L.Diffuse, diffuseFactor);
        specular = ToonShadingForSpecular(mat.Specular * L.Specular, specularFactor);
    }
}

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
                       out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVec = L.Position - pos;
    
    float d = length(lightVec);
    
    if (d > L.Range)
        return;
    
    lightVec /= d;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v              = reflect(-lightVec, normal);
        float  specularFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        
        diffuse  = diffuseFactor * mat.Diffuse * L.Diffuse;
        specular = specularFactor * mat.Specular * L.Specular;
    }
    
    float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));
    
    diffuse  *= att;
    specular *= att;
}

void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
                      out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightVec = L.Position - pos;
    
    float d = length(lightVec);
    
    if (d > L.Range)
        return;
    
    lightVec /= d;
    
    ambient = mat.Ambient * L.Ambient;
    
    float diffuseFactor = dot(lightVec, normal);
    
    [flatten]
    if (diffuseFactor > 0.0f)
    {
        float3 v              = reflect(-lightVec, normal);
        float  specularFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
        
        //diffuse  = diffuseFactor * mat.Diffuse * L.Diffuse;
        //specular = specularFactor * mat.Specular * L.Specular;
        
        diffuse = ToonShadingForDiffuse(mat.Diffuse * L.Diffuse, diffuseFactor);
        specular = ToonShadingForSpecular(mat.Specular * L.Specular, specularFactor);
    }
    
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
    
    float att = spot / dot(L.Att, float3(1.0f, d, d * d));

    ambient  *= spot;
    diffuse  *= att;
    specular *= att;
}