// perform gaussian blurs on shadow maps

#include "Quad.fx"

Texture2D g_shadowMap : register(t0);

cbuffer GaussianShadow : register(b0) {
	float2 rcpScreenSize;
};

static const int KERNEL_WIDTH = 7;
static const float KERNEL[] = { 0.0045, 0.0540, 0.2420, 0.3990, 0.2420, 0.0540, 0.0045 };
static const int KERNEL_START = -(KERNEL_WIDTH - 1) / 2;
static const int KERNEL_END = -KERNEL_START;

float4 PS_Vertical(PS_INPUT input) : SV_Target {
	float x = input.texCoord.x;
	float2 result = 0.0;
	for (int i = KERNEL_START; i <= KERNEL_END; i++) {
		float y = input.texCoord.y + i * rcpScreenSize.y;
		result += KERNEL[i - KERNEL_START] * g_shadowMap.SampleLevel(g_samPoint, float2(x, y), 0).rg;
	}
	return float4(result, 1.0, 1.0);
}

float4 PS_Horizontal(PS_INPUT input) : SV_Target {
	float y = input.texCoord.y;
	float2 result = 0.0;
	for (int i = KERNEL_START; i <= KERNEL_END; i++) {
		float x = input.texCoord.x + i * rcpScreenSize.x;
		result += KERNEL[i - KERNEL_START] * g_shadowMap.SampleLevel(g_samPoint, float2(x, y), 0).rg;
	}
	return float4(result, 1.0, 1.0);
}
