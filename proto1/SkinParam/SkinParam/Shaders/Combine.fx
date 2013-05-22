// Combine shader

#include "Quad.fx"
#include "Lum.fx"

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
	
	return float4(clamp(color * albedo + specular, 0, 1e30), 1.0);
}

float4 PS_AA(PS_INPUT input) : SV_Target {
	float3 color = PS(input).rgb;
	return lum_output_linear(color);
}
