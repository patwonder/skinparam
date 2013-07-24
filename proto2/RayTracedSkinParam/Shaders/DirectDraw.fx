// Direct texture draw on screen

#include "Quad.fx"

Texture2D g_texture : register(t0);

float4 PS_Point(PS_INPUT input) : SV_Target {
	return g_texture.SampleLevel(g_samPoint, input.texCoord, 0);
}

float4 PS_Linear(PS_INPUT input) : SV_Target {
	return g_texture.SampleLevel(g_samLinear, input.texCoord, 0);
}

float4 PS_Point_UpsideDown(PS_INPUT input) : SV_Target {
	return g_texture.SampleLevel(g_samPoint, float2(input.texCoord.x, 1.0 - input.texCoord.y), 0);
}

float4 PS_Linear_UpsideDown(PS_INPUT input) : SV_Target {
	return g_texture.SampleLevel(g_samLinear, float2(input.texCoord.x, 1.0 - input.texCoord.y), 0);
}
