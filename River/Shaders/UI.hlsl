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
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorld);
	
    vout.TexC = vin.TexC;
	
    vout.Color = float4(vin.Color.r / 255.f, vin.Color.g / 255.f, vin.Color.b / 255.f, vin.Color.a / 255.f); //vin.Color / 255;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return gTextureMaps[6].Sample(gsamLinearWrap, pin.TexC);
    //return pin.Color;
    //return float4(gSsaoMap.Sample(gsamLinearWrap, pin.TexC).rrr, 1.0f);
}