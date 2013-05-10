// perform gaussian blurring on the screen quad

#include "Quad.fx"

// r, g, b: irradiance color; a: stencil
Texture2D g_screenTexture : register(t0);
// r, g : kernelWidth in x, y direction; b : depth; a: stencil
Texture2D g_diffuseStencilTexture : register(t1);

cbuffer Gaussian : register(b0) {
	float g_blurWidth; // blur width
	float g_invAspectRatio; // inverse of aspect ratio (height / width)
}

static const uint KERNEL_SIZE = 7;
static const float KERNEL[] = { 0.383, 0.006, 0.061, 0.242, 0.242, 0.061, 0.006 };
static const float KERNEL_DX[] = {  0.0, -3.0, -2.0, -1.0, 1.0, 2.0, 3.0 };

float4 PS_Vertical(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthY = diffuseStencil.y;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthY;

		float4 finalColor = color * KERNEL[0];
		// do the vertical blurring
		float x = input.texCoord.x;
		for (uint i = 1; i < KERNEL_SIZE; i++) {
			float y = kernelWidth * KERNEL_DX[i] + input.texCoord.y;
			finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL[i];
		}

		finalColor /= finalColor.a;
		return finalColor;
	}
}

float4 PS_Horizontal(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthX = diffuseStencil.x;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthX * g_invAspectRatio;

		float4 finalColor = color * KERNEL[0];
		// do the horizontal blurring
		float y = input.texCoord.y;
		for (uint i = 1; i < KERNEL_SIZE; i++) {
			float x = kernelWidth * KERNEL_DX[i] + input.texCoord.x;
			finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL[i];
		}

		finalColor /= finalColor.a;
		return finalColor;
	}
}
