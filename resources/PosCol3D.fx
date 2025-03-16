//----------------------------------------
//  Global variable
//----------------------------------------
float4x4 gWorldViewProjection : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

const float3 gLightDirection =-float3(.577f, -.577f, .577f);
float4x4 gWorldMatrix : WORLD;
float3 gCameraPosition : CAMERA;

const float gPI = 3.14159265358979323846264338327950288f;

const float3 gAmbient = { .025f,.025f,.025f};
const float gLightIntensity = 7.f;
const float gShininess = 25.f;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

BlendState gBlendState{};
DepthStencilState gDepthStencilState{};

//----------------------------------------
//  Input/Output Structs
//----------------------------------------

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION0;
    float4 worldPosition : WORLD;
    float2 uv : TEXCOORD;
    float3 normal: NORMAL;
    float3 tangent : TANGENT;
};


//----------------------------------------
//  Functions
//----------------------------------------
float4 SampleDiffuseTexture(SamplerState Sampler, float2 uv)
{
    return gDiffuseMap.Sample(Sampler, uv);
}
float4 SampleNormalTexture(SamplerState Sampler, float2 uv)
{
    return gNormalMap.Sample(Sampler, uv);
}
float4 SampleSpecularTexture(SamplerState Sampler, float2 uv)
{
    return gSpecularMap.Sample(Sampler, uv);
}
float4 SampleGlossinessTexture(SamplerState Sampler, float2 uv)
{
    return gGlossinessMap.Sample(Sampler, uv);
}

float3 ObservedAreaShading(VS_OUTPUT input, float3 normal) // Lambert Cosine
{
    float lambertsCos = dot(normal, gLightDirection);
       lambertsCos = clamp(lambertsCos, 0.f, 1.f);
    
       return lambertsCos.rrr;
}
float3 DiffuseShading(VS_OUTPUT input, float4 diffuseSample,float3 normal) // Lambert diffuse
{
   float lambertsCos = ObservedAreaShading(input, normal).r;
   
   float3 lightContribution = (lambertsCos * gLightIntensity * diffuseSample.rgb) / gPI;
   
   return lightContribution;
}
float3 SpecularShading(VS_OUTPUT input, float4 specularSample, float4 glossinessSample, float3 normal)   // Phong specular
{
    float specularReflectionCoefficient = specularSample.r; // Since these textures store their values in grey scale, we can use the red channel only (optimized)
    float phongExponent = glossinessSample.r * gShininess; // Since these textures store their values in grey scale, we can use the red channel only (optimized)
    
    if (specularReflectionCoefficient > 0.0f)
    {
        float3 invViewDirection = normalize(gCameraPosition - input.worldPosition.xyz);
        
        float3 reflectVector = normalize(gLightDirection - (2 * dot(normal, gLightDirection) * normal));
        float cosAlpha = dot(reflectVector, -invViewDirection);
        cosAlpha = max(cosAlpha, 0.f);
    
        float3 lightContribution = specularReflectionCoefficient * (pow(cosAlpha, phongExponent)) * float3(1.f, 1.f, 1.f);
            
        return lightContribution;
    }
	else 
        return float3(0.f, 0.f, 0.f);
}

float4 CombinedShading(VS_OUTPUT input, 
        float4 diffuseSample, float4 specularSample, float4 glossinessSample,
        float3 normal)   //Combined SHADINF
{
    // Lambert diffuse
    float3 lightContribution = DiffuseShading(input, diffuseSample, normal);
    // Phong
    lightContribution += SpecularShading(input, specularSample, glossinessSample,normal);
    
    // Ambient
    lightContribution += gAmbient;
    
    return float4(lightContribution.r, lightContribution.g, lightContribution.b, 1.f);
}
//----------------------------------------
//  Vertex Shader
//----------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    // Transform input worldPosition by the World matrix
    output.worldPosition = mul(float4(input.position, 1.0f), gWorldMatrix);

    // Transform input position by the World-View-Projection matrix
    output.position = mul(float4(input.position, 1.0f), gWorldViewProjection);

    // Pass UVs
    output.uv = input.uv;

    // Compute tangent space components
    output.normal = mul(float4(input.normal, 0.0f), gWorldMatrix).xyz; // World-space normal
    output.tangent = mul(float4(input.tangent, 0.0f), gWorldMatrix).xyz; // World-space tangent

    return output;
}

//----------------------------------------
//  Pixel Shader
//----------------------------------------
float4 PS_Point(VS_OUTPUT input) : SV_Target
{
    float4 finalColor = float4(0.f, 0.f, 0.f, 1.f); // Initialize final color

    // Tangent-to-World matrix without explicitly passing the bitangent
    float3 biNormal = cross(input.normal, input.tangent); // Compute bitangent
    float3x3 tangentSpace = float3x3(
        normalize(input.tangent),
        normalize(biNormal),
        normalize(input.normal)
    );

    // Sample the normal map (range [0,1]), remap to [-1,1]
    float3 sampledNormal = SampleNormalTexture(samPoint, input.uv).rgb * 2.0f - 1.0f;

    // Transform the normal to world space
    float3 worldNormal = normalize(mul(sampledNormal, tangentSpace));

    // Replace the original normal in your shading calculations
    if (ObservedAreaShading(input, worldNormal).r > 0.f)
    {
        finalColor = CombinedShading(input,
              SampleDiffuseTexture(     samPoint, input.uv),
              SampleSpecularTexture(    samPoint, input.uv),
              SampleGlossinessTexture(  samPoint, input.uv),
              worldNormal);
    }
    
    return finalColor;
}

float4 PS_Linear(VS_OUTPUT input) : SV_Target
{
    float4 finalColor = float4(0.f, 0.f, 0.f, 1.f); // Initialize final color

    // Tangent-to-World matrix without explicitly passing the bitangent
    float3 biNormal = cross(input.normal, input.tangent); // Compute bitangent
    float3x3 tangentSpace = float3x3(
        normalize(input.tangent),
        normalize(biNormal),
        normalize(input.normal)
    );

    // Sample the normal map (range [0,1]), remap to [-1,1]
    float3 sampledNormal = SampleNormalTexture(samPoint, input.uv).rgb * 2.0f - 1.0f;

    // Transform the normal to world space
    float3 worldNormal = normalize(mul(sampledNormal, tangentSpace));

    // Replace the original normal in your shading calculations
    if (ObservedAreaShading(input, worldNormal).r > 0.f)
    {
        finalColor = CombinedShading(input,
              SampleDiffuseTexture(samLinear, input.uv),
              SampleSpecularTexture(samLinear, input.uv),
              SampleGlossinessTexture(samLinear, input.uv),
              worldNormal);
    }
    
    return finalColor;
}

float4 PS_Anisotropic(VS_OUTPUT input) : SV_Target
{
    
    float4 finalColor = float4(0.f, 0.f, 0.f, 1.f); // Initialize final color

    // Tangent-to-World matrix without explicitly passing the bitangent
    float3 biNormal = cross(input.normal, input.tangent); // Compute bitangent
    float3x3 tangentSpace = float3x3(
        normalize(input.tangent),
        normalize(biNormal),
        normalize(input.normal)
    );

    // Sample the normal map (range [0,1]), remap to [-1,1]
    float3 sampledNormal = SampleNormalTexture(samPoint, input.uv).rgb * 2.0f - 1.0f;

    // Transform the normal to world space
    float3 worldNormal = normalize(mul(sampledNormal, tangentSpace));

    // Replace the original normal in your shading calculations
    if (ObservedAreaShading(input, worldNormal).r > 0.f)
    {
        finalColor = CombinedShading(input,
              SampleDiffuseTexture(samAnisotropic, input.uv),
              SampleSpecularTexture(samAnisotropic, input.uv),
              SampleGlossinessTexture(samAnisotropic, input.uv),
              worldNormal);
    }
    
    return finalColor;
}


//----------------------------------------
//  Technique
//----------------------------------------
technique11 PointTechnique
{
    pass P0
    {
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Point()));
    }
}

technique11 LinearTechnique
{
    pass P0
    {
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Linear()));
    }
}

technique11 AnisotropicTechnique
{
    pass P0
    {
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Anisotropic()));
    }
}

