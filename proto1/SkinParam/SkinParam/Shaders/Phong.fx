// Implements the phong shading model

#include "Transform.fx"

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};

VS_OUTPUT VS(float4 pos : POSITION, float4 color : COLOR) {
	VS_OUTPUT output;
	output.pos = mul(pos, g_matWorld);
	output.pos = mul(output.pos, g_matViewProj);
	output.color = color;
	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target {
	return input.color;
}
