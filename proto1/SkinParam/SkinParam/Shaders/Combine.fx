// Combine shader

#include "Quad.fx"

static const uint NUM_GAUSSIANS = 6;
Texture2D g_albedo : register(t0);
Texture2D g_specular : register(t1);
Texture2D g_gaussians[NUM_GAUSSIANS] : register(t2);

cbuffer Combine : register(b0) {
	float3 g_weights[NUM_GAUSSIANS];
}

float4 PS(PS_INPUT input) : SV_Target {
	float3 specular = g_specular.SampleLevel(g_samPoint, input.texCoord, 0).rgb;
	float3 albedo = g_albedo.SampleLevel(g_samPoint, input.texCoord, 0).rgb;

	float3 color = 0.0;
	for (uint i = 0; i < NUM_GAUSSIANS; i++) {
		color += g_weights[i] * g_gaussians[i].SampleLevel(g_samPoint, input.texCoord, 0).rgb;
	}
	
	return float4(clamp(color * albedo + specular, 0, 2.0), 1.0);
}

float4 PS_AA(PS_INPUT input) : SV_Target {
	float3 color = saturate(PS(input).rgb);
	return float4(color, sqrt(dot(color, float3(0.299, 0.587, 0.114))));
}
