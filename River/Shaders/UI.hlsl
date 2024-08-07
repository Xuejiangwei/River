#include "Common.hlsl" 

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
    uint4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;

    // Already in homogeneous clip space.
    //vout.PosH = float4(vin.PosL, 1.0f);
    vin.PosL.x = (vin.PosL.x / gRenderTargetSize.x) * 2 - 1;
    vin.PosL.y = -(vin.PosL.y / gRenderTargetSize.y) * 2 + 1;
    
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorld);
	
    vout.TexC = vin.TexC;
	
    vout.Color = float4(vin.Color.r / 255.f, vin.Color.g / 255.f, vin.Color.b / 255.f, 0.5f); //vin.Color / 255;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    //MaterialData matData = gMaterialData[gMaterialIndex];
    //uint diffuseMapIndex = matData.DiffuseMapIndex;
    //float3 color = gTextureMaps[0].Sample(gsamLinearWrap, pin.TexC).rgb;
    
    if (gObjPad0 == 1 << 0)
    {
       return pin.Color;
    }
    else
    {
        float4 color = gTextureMaps[0].Sample(gsamLinearWrap, pin.TexC);
        clip(color.a - 0.1f);
        return color; //float4(color, pin.Color.a);
    }
    
    //return float4(gSsaoMap.Sample(gsamLinearWrap, pin.TexC).rrr, 1.0f);
    //return gShadowMap.Sample(gsamLinearWrap, pin.TexC);
}