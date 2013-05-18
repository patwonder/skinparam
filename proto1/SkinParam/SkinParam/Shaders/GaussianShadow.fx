// perform gaussian blurs on shadow maps

#include "Quad.fx"

Texture2D g_shadowMap : register(t0);

cbuffer GaussianShadow : register(b0) {
	float2 rcpScreenSize;
};

// Standard Deviation = 1.0
static const int KERNEL_WIDTH = 7;
static const float KERNEL[] = { 0.0062, 0.0606, 0.2417, 0.3830, 0.2417, 0.0606, 0.0062 };

// Standard Deviation = 0.9
//static const int KERNEL_WIDTH = 7;
//static const float KERNEL[] = { 0.0027, 0.0451, 0.2415, 0.4214, 0.2415, 0.0451, 0.0027 };

// Standard Deviation ~ 0.9 (slightly smaller)
//static const int KERNEL_WIDTH = 5;
//static const float KERNEL[] = { 0.0478, 0.2415, 0.4214, 0.2415, 0.0478 };

// Standard Deviation = 0.8
//static const int KERNEL_WIDTH = 5;
//static const float KERNEL[] = {  0.0304, 0.2356, 0.4680, 0.2356, 0.0304 };

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
