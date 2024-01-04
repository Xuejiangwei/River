#pragma once

#include "RiverHead.h"
#define MaxLights 16

class RenderPass
{
public:
	struct Light
	{
		Float3 Strength = { 0.5f, 0.5f, 0.5f };
		float FalloffStart = 1.0f;                          // point/spot light only
		Float3 Direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
		float FalloffEnd = 10.0f;                           // point/spot light only
		Float3 Position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
		float SpotPower = 64.0f;                            // spot light only
	};

	struct PassUniform
	{
		Matrix4x4 View = Matrix4x4::UnitMatrix();
		Matrix4x4 InvView = Matrix4x4::UnitMatrix();
		Matrix4x4 Proj = Matrix4x4::UnitMatrix();
		Matrix4x4 InvProj = Matrix4x4::UnitMatrix();
		Matrix4x4 ViewProj = Matrix4x4::UnitMatrix();
		Matrix4x4 InvViewProj = Matrix4x4::UnitMatrix();
		Matrix4x4 ViewProjTex = Matrix4x4::UnitMatrix();
		Matrix4x4 ShadowTransform = Matrix4x4::UnitMatrix();
		Float3 EyePosW = { 0.0f, 0.0f, 0.0f };
		float cbPerObjectPad1 = 0.0f;
		Float2 RenderTargetSize = { 0.0f, 0.0f };
		Float2 InvRenderTargetSize = { 0.0f, 0.0f };
		float NearZ = 0.0f;
		float FarZ = 0.0f;
		float TotalTime = 0.0f;
		float DeltaTime = 0.0f;

		Float4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

		Light Lights[MaxLights];
	};

public:
	RenderPass();

	virtual ~RenderPass();

	virtual void Render() = 0;

private:

};
