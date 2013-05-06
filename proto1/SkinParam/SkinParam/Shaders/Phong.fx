// Implements the phong shading model

#include "Lighting.fx"
#include "Bump.fx"

struct VS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 worldPos : WORLDPOS;
	float2 texCoord : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output;
	float4 wpos4 = mul(input.pos, g_matWorld);
	output.normal = normalize(mul(float4(input.normal, 0.0), g_matWorld).xyz);
	output.tangent = normalize(mul(float4(input.tangent, 0.0), g_matWorld).xyz);
	output.binormal = normalize(mul(float4(input.binormal, 0.0), g_matWorld).xyz);

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

	// calculated pertubated normal
	float3 normal = calcNormalWS(g_samBump, g_bump, input.texCoord, input.normal, input.tangent, input.binormal);
	
	// shading
	float3 color = phong_lighting(g_material, g_ambient, g_lights, g_posEye, input.worldPos, tex * input.color.rgb, normal);
	return float4(color, 1.0);
}
