//----------------------------------------
//  Global variable
//----------------------------------------
float4x4 gWorldViewProjection : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;

float4x4 gWorldMatrix : WORLD;
float3 gCameraPosition : CAMERA;

SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};


RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockWise = false;  //default
};

BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;

    //others are redundant because
    //  StencilEnable is FALSE
    //(for demo purposes only)
    StencilReadMask = 0x0f;
    StencilWriteMask = 0x0f;

    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;

    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
    
};

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


float4 FlatShading(VS_OUTPUT input, float4 diffuseSample) // Flat diffuse
{
    return diffuseSample;
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
   
    output.uv = input.uv;
    output.normal = mul(float4(input.normal, 1.0f), gWorldMatrix);
    output.tangent = mul(float4(input.tangent, 1.0f), gWorldMatrix);

    return output;
}

//----------------------------------------
//  Pixel Shader
//----------------------------------------

float4 PS(VS_OUTPUT input) : SV_Target
{
    return FlatShading(input, SampleDiffuseTexture(samPoint, input.uv));
}


//----------------------------------------
//  Technique
//----------------------------------------
technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
