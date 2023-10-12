//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include common HLSL code.
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
    uint4 Color : COLOR;
	float2 TexC    : TEXCOORD;
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
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosH = mul(posW, gViewProj);
	
    vout.TexC = vin.TexC;
	
    vout.Color = float4(vin.Color.r / 255.f, vin.Color.g / 255.f, vin.Color.b / 255.f, vin.Color.a / 255.f); //vin.Color / 255;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color; //gTextureMaps[6].Sample(gsamLinearWrap, pin.TexC);
}


