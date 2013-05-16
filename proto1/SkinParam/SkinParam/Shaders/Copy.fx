// Shader that just copy pixels from source to destination

#include "Quad.fx"

Texture2D g_screenTexture : register(t0);

float4 PS(PS_INPUT input) : SV_Target {
	return g_screenTexture.Sample(g_samPoint, input.texCoord);
}

float4 PS_Linear(PS_INPUT input) : SV_Target {
	return g_screenTexture.Sample(g_samLinear, input.texCoord);
}
