// Simulate drawing on a canvas
// with (0, 0) as the top-left corner
// and (1, 1) as the bottom-right corner

#include "Lum.fx"

struct VS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct PS_INPUT {
	float4 pos : SV_Position;
	float4 color : COLOR;
};

PS_INPUT VS(VS_INPUT input) {
	PS_INPUT output;

	// convert to screen-space
	float2 vPosPS = float2(2.0, -2.0) * (input.pos.xy - 0.5);
	output.pos = float4(vPosPS, 0.0, 1.0);
	output.color = input.color;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target {
	return float4(input.color.rgb, 1.0);
}

float4 PS_AA(PS_INPUT input) : SV_Target {
	float3 color = PS(input).rgb;
	return lum_output_linear(color);
}
