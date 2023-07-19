struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer cbPerObject : register(b0)
{
    float4 test;
};
 
PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;
 
    result.position = position;
    result.color = test;
 
    return result;
}
 
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}