// perform gaussian blurring on the screen quad

#include "Quad.fx"

// r, g, b: irradiance color; a: specular color
Texture2D g_screenTexture : register(t0);
// r, g : diffuse NdotV in x, y direction; b : depth; a: stencil
Texture2D g_diffuseStencilTexture : register(t1);

cbuffer Gaussian : register(b0) {
	float g_blurWidth; // blur width
	float g_invAspectRatio; // inverse of aspect ratio (height / width)
}

static const uint KERNEL_SIZE = 7;
static const float KERNEL[] = { 0.383, 0.006, 0.061, 0.242, 0.242, 0.061, 0.006 };
static const float KERNEL_DX[] = {  0.0, -3.0, -2.0, -1.0, 1.0, 2.0, 3.0 };

// constants for kernel width calculation
static const float DEPTH_NEAR = 0.1;
static const float DEPTH_FAR = 30.0;
// emprical value : in texture space, 0.001 per mm at depth 10
float SIZE_ALPHA = 0.01;

// converts from projection space depth to camera space depth
float getDepthCS(float depthPS) {
	return lerp(DEPTH_NEAR, DEPTH_FAR, depthPS);
}
float getKernelSize(float depthPS, float grad) {
	return SIZE_ALPHA / getDepthCS(depthPS) * grad;
}

float4 PS_Vertical(PS_INPUT input) : SV_Target {
	float4 diffuseStencil = g_diffuseStencilTexture.Sample(g_samLinear, input.texCoord);
	float4 sample = g_screenTexture.Sample(g_samLinear, input.texCoord);
	float3 color = sample.rgb;
	float diffuseY = diffuseStencil.g;
	float depth = diffuseStencil.b;
	float stencil = diffuseStencil.a;
	// kernel width for each channel
	float kernelWidth = g_blurWidth * getKernelSize(depth, diffuseY);

	float3 finalColor = color * KERNEL[0];
	// do the horizontal blurring
	float x = input.texCoord.x;
	for (uint i = 1; i < KERNEL_SIZE; i++) {
		float y = kernelWidth * KERNEL_DX[i] + input.texCoord.y;
		float3 rgb = g_screenTexture.Sample(g_samLinear, float2(x, y)).rgb;
		finalColor += rgb * KERNEL[i];
	}

	// should retain original alpha(depth) value
	return float4(lerp(color, finalColor, stencil), 1.0);
}

float4 PS_Horizontal(PS_INPUT input) : SV_Target {
	float4 diffuseStencil = g_diffuseStencilTexture.Sample(g_samLinear, input.texCoord);
	float4 sample = g_screenTexture.Sample(g_samLinear, input.texCoord);
	float3 color = sample.rgb;
	float diffuseX = diffuseStencil.r;
	float depth = diffuseStencil.b;
	float stencil = diffuseStencil.a;
	// kernel width for each channel
	float kernelWidth = g_blurWidth * getKernelSize(depth, diffuseX) * g_invAspectRatio;

	float3 finalColor = color * KERNEL[0];
	// do the horizontal blurring
	float y = input.texCoord.y;
	for (uint i = 1; i < KERNEL_SIZE; i++) {
		float x = kernelWidth * KERNEL_DX[i] + input.texCoord.x;
		float3 rgb = g_screenTexture.Sample(g_samLinear, float2(x, y)).rgb;
		finalColor += rgb * KERNEL[i];
	}

	// should retain original alpha(depth) value
	return float4(lerp(color, finalColor, stencil), 1.0);
}
