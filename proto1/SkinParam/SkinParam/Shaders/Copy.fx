// Shader that just copy pixels from source to destination

#include "Quad.fx"

Texture2D g_screenTexture : register(t0);

cbuffer Copy : register(b0) {
	float4 scaleFactor;
	float4 defaultValue;
	float4 lerps;
};

float4 PS(PS_INPUT input) : SV_Target {
	return lerp(defaultValue, scaleFactor * g_screenTexture.Sample(g_samPoint, input.texCoord), lerps);
}

float4 PS_Linear(PS_INPUT input) : SV_Target {
	return lerp(defaultValue, scaleFactor * g_screenTexture.Sample(g_samLinear, input.texCoord), lerps);
}
