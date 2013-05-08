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
	float3 specular = g_specular.Sample(g_samPoint, input.texCoord).rgb;
	float3 albedo = g_albedo.Sample(g_samPoint, input.texCoord).rgb;

	float3 color = 0.0;
	for (uint i = 0; i < NUM_GAUSSIANS; i++) {
		color += g_weights[i] * g_gaussians[i].Sample(g_samPoint, input.texCoord).rgb;
	}
	
	return float4(color * albedo + specular, 1.0);
}
