// Combine bloom results

#include "Quad.fx"

static const uint BLOOM_PASSES = 6;

Texture2D g_screenTexture : register(t0);
Texture2D g_bloom[BLOOM_PASSES] : register(t1);

float4 PS(PS_INPUT input) : SV_Target {
	float3 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0).rgb;
	float3 finalColor = color;// + g_bloom[0].SampleLevel(g_samPoint, input.texCoord, 0).rgb;
	for (uint i = 1; i < BLOOM_PASSES; i++) {
		finalColor += g_bloom[i].SampleLevel(g_samLinear, input.texCoord, 0).rgb / (1 << i);
	}
	return float4(finalColor, 1.0);
}

float4 PS_AA(PS_INPUT input) : SV_Target {
	float3 color = saturate(PS(input).rgb);
	return float4(color, sqrt(dot(color, float3(0.299, 0.587, 0.114))));
}
