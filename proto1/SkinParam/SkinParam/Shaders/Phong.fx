// Implements the phong shading model

#include "Lighting.fx"

struct VS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : TEXCOORD0;
	float3 worldPos : TEXCOORD1;
	float2 texCoord : TEXCOORD2;
};

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output;
	float4 wpos4 = mul(input.pos, g_matWorld);
	output.normal = normalize(mul(float4(input.normal, 0.0), g_matWorld).xyz);

	float bumpAmount = g_material.bump_multiplier * 2 * (g_bump.SampleLevel(g_samBump, input.texCoord, 0).r - 0.5);
	wpos4.xyz += wpos4.w * bumpAmount * output.normal;

	output.worldPos = wpos4.xyz / wpos4.w;
	output.pos = mul(wpos4, g_matViewProj);
	output.color = input.color;
	output.texCoord = input.texCoord;
	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target {
	// texture
	float3 tex = g_texture.Sample(g_samTexture, input.texCoord).rgb;
	float3 color = phong_lighting(g_material, g_ambient, g_lights, g_posEye, input.worldPos, tex * input.color.rgb, input.normal);
	return float4(color, 1.0);
}
