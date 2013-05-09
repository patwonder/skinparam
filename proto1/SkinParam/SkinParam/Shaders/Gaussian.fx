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
static const float SIZE_ALPHA = 0.015;

// converts from projection space depth to camera space depth
float getDepthCS(float depthPS) {
	return lerp(DEPTH_NEAR, DEPTH_FAR, depthPS);
}
float getKernelSize(float depthPS, float grad) {
	return SIZE_ALPHA / getDepthCS(depthPS) * grad;
}

static const float DD_START = 0.0005;
static const float DD_LEN = 0.0005;

float4 PS_Vertical(PS_INPUT input) : SV_Target {
	float4 diffuseStencil = g_diffuseStencilTexture.Sample(g_samPoint, input.texCoord);
	float4 sample = g_screenTexture.Sample(g_samPoint, input.texCoord);
	float3 color = sample.rgb;
	float diffuseY = diffuseStencil.y;
	float depth = diffuseStencil.z;
	diffuseY *= saturate(1 - (abs(ddy(depth)) - DD_START) / DD_LEN);
	float stencil = diffuseStencil.w;
	// kernel width for each channel
	float kernelWidth = g_blurWidth * getKernelSize(depth, diffuseY);

	float3 finalColor = color * KERNEL[0];
	float totalStencil = KERNEL[0]; // avoid blurring into the background
	// do the vertical blurring
	float x = input.texCoord.x;
	for (uint i = 1; i < KERNEL_SIZE; i++) {
		float y = kernelWidth * KERNEL_DX[i] + input.texCoord.y;
		float3 rgb = g_screenTexture.Sample(g_samLinear, float2(x, y)).rgb;
		finalColor += rgb * KERNEL[i];
		totalStencil += g_diffuseStencilTexture.Sample(g_samPoint, float2(x, y)).w * KERNEL[i];
	}

	return float4(lerp(color, finalColor / totalStencil, stencil), 1.0);
}

float4 PS_Horizontal(PS_INPUT input) : SV_Target {
	float4 diffuseStencil = g_diffuseStencilTexture.Sample(g_samPoint, input.texCoord);
	float4 sample = g_screenTexture.Sample(g_samPoint, input.texCoord);
	float3 color = sample.rgb;
	float diffuseX = diffuseStencil.x;
	float depth = diffuseStencil.z;
	diffuseX *= saturate(1 - (abs(ddx(depth)) - DD_START) / DD_LEN);
	float stencil = diffuseStencil.w;
	// kernel width for each channel
	float kernelWidth = g_blurWidth * getKernelSize(depth, diffuseX) * g_invAspectRatio;

	float3 finalColor = color * KERNEL[0];
	float totalStencil = KERNEL[0]; // avoid blurring into the background
	// do the horizontal blurring
	float y = input.texCoord.y;
	for (uint i = 1; i < KERNEL_SIZE; i++) {
		float x = kernelWidth * KERNEL_DX[i] + input.texCoord.x;
		float3 rgb = g_screenTexture.Sample(g_samLinear, float2(x, y)).rgb;
		finalColor += rgb * KERNEL[i];
		totalStencil += g_diffuseStencilTexture.Sample(g_samPoint, float2(x, y)).w * KERNEL[i];
	}

	return float4(lerp(color, finalColor / totalStencil, stencil), 1.0);
}
