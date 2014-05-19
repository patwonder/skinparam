
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

// perform gaussian blurring on the screen quad

#include "Quad.fx"

// r, g, b: irradiance color; a: stencil
Texture2D g_screenTexture : register(t0);
// r, g : kernelWidth in x, y direction; b : depth; a: stencil
Texture2D g_diffuseStencilTexture : register(t1);

cbuffer Gaussian : register(b0) {
	float g_blurWidth; // blur width
	float g_invAspectRatio; // inverse of aspect ratio (height / width)
	float g_screenWidth;
	float g_screenHeight;
}

static const float KERNEL_SIZE_7_THRESHOLD = 0.873;
static const uint KERNEL_SIZE_7 = 7;
static const float KERNEL_7[] = { 0.3830, 0.0062, 0.0606, 0.2417, 0.2417, 0.0606, 0.0062 };
static const float KERNEL_DX_7[] = {  0.0, -3.0, -2.0, -1.0, 1.0, 2.0, 3.0 };

static const float KERNEL_SIZE_5_THRESHOLD = 0.593;
static const uint KERNEL_SIZE_5 = 5;
static const float KERNEL_5[] = { 0.4680, 0.0304, 0.2356, 0.2356, 0.0304 };
static const float KERNEL_DX_5[] = {  0.0, -2.5, -1.25, 1.25, 2.5 };

static const float KERNEL_SIZE_3_THRESHOLD = 0.1;
static const uint KERNEL_SIZE_3 = 3;
static const float KERNEL_3[] = { 0.6826, 0.1587, 0.1587 };
static const float KERNEL_DX_3[] = {  0.0, -2.0, 2.0 };

float4 PS_Vertical_Core_7(float4 color, float kernelWidth, float2 texCoord) {
	float4 finalColor = color * KERNEL_7[0];
	// do the vertical blurring
	float x = texCoord.x;
	for (uint i = 1; i < KERNEL_SIZE_7; i++) {
		float y = kernelWidth * KERNEL_DX_7[i] + texCoord.y;
		finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL_7[i];
	}

	finalColor /= finalColor.a;
	return finalColor;
}

float4 PS_Horizontal_Core_7(float4 color, float kernelWidth, float2 texCoord) {
	float4 finalColor = color * KERNEL_7[0];
	// do the horizontal blurring
	float y = texCoord.y;
	for (uint i = 1; i < KERNEL_SIZE_7; i++) {
		float x = kernelWidth * KERNEL_DX_7[i] + texCoord.x;
		finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL_7[i];
	}

	finalColor /= finalColor.a;
	return finalColor;
}

float4 PS_Vertical_Core_5(float4 color, float kernelWidth, float2 texCoord) {
	float4 finalColor = color * KERNEL_5[0];
	// do the vertical blurring
	float x = texCoord.x;
	for (uint i = 1; i < KERNEL_SIZE_5; i++) {
		float y = kernelWidth * KERNEL_DX_5[i] + texCoord.y;
		finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL_5[i];
	}

	finalColor /= finalColor.a;
	return finalColor;
}

float4 PS_Horizontal_Core_5(float4 color, float kernelWidth, float2 texCoord) {
	float4 finalColor = color * KERNEL_5[0];
	// do the horizontal blurring
	float y = texCoord.y;
	for (uint i = 1; i < KERNEL_SIZE_5; i++) {
		float x = kernelWidth * KERNEL_DX_5[i] + texCoord.x;
		finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL_5[i];
	}

	finalColor /= finalColor.a;
	return finalColor;
}

float4 PS_Vertical_Core_3(float4 color, float kernelWidth, float2 texCoord) {
	float4 finalColor = color * KERNEL_3[0];
	// do the vertical blurring
	float x = texCoord.x;
	for (uint i = 1; i < KERNEL_SIZE_3; i++) {
		float y = kernelWidth * KERNEL_DX_3[i] + texCoord.y;
		finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL_3[i];
	}

	finalColor /= finalColor.a;
	return finalColor;
}

float4 PS_Horizontal_Core_3(float4 color, float kernelWidth, float2 texCoord) {
	float4 finalColor = color * KERNEL_3[0];
	// do the horizontal blurring
	float y = texCoord.y;
	for (uint i = 1; i < KERNEL_SIZE_3; i++) {
		float x = kernelWidth * KERNEL_DX_3[i] + texCoord.x;
		finalColor += g_screenTexture.SampleLevel(g_samLinear, float2(x, y), 0) * KERNEL_3[i];
	}

	finalColor /= finalColor.a;
	return finalColor;
}

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
		
		return PS_Vertical_Core_7(color, kernelWidth, input.texCoord);
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

		return PS_Horizontal_Core_7(color, kernelWidth, input.texCoord);
	}
}

float4 PS_Vertical_7(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthY = diffuseStencil.y;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthY;

		// dispatcher
		float widthPixels = kernelWidth * g_screenHeight;
		if (widthPixels > KERNEL_SIZE_7_THRESHOLD) {
			return PS_Vertical_Core_7(color, kernelWidth, input.texCoord);
		} else if (widthPixels > KERNEL_SIZE_5_THRESHOLD) {
			return PS_Vertical_Core_5(color, kernelWidth, input.texCoord);
		} else if (widthPixels > KERNEL_SIZE_3_THRESHOLD) {
			return PS_Vertical_Core_3(color, kernelWidth, input.texCoord);
		} else {
			return color;
		}
	}
}

float4 PS_Horizontal_7(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthX = diffuseStencil.x;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthX * g_invAspectRatio;

		// dispatcher
		float widthPixels = kernelWidth * g_screenWidth;
		if (widthPixels > KERNEL_SIZE_7_THRESHOLD) {
			return PS_Horizontal_Core_7(color, kernelWidth, input.texCoord);
		} else if (widthPixels > KERNEL_SIZE_5_THRESHOLD) {
			return PS_Horizontal_Core_5(color, kernelWidth, input.texCoord);
		} else if (widthPixels > KERNEL_SIZE_3_THRESHOLD) {
			return PS_Horizontal_Core_3(color, kernelWidth, input.texCoord);
		} else {
			return color;
		}
	}
}

float4 PS_Vertical_5(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthY = diffuseStencil.y;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthY;

		// dispatcher
		float widthPixels = kernelWidth * g_screenHeight;
		if (widthPixels > KERNEL_SIZE_5_THRESHOLD) {
			return PS_Vertical_Core_5(color, kernelWidth, input.texCoord);
		} else if (widthPixels > KERNEL_SIZE_3_THRESHOLD) {
			return PS_Vertical_Core_3(color, kernelWidth, input.texCoord);
		} else {
			return color;
		}
	}
}

float4 PS_Horizontal_5(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthX = diffuseStencil.x;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthX * g_invAspectRatio;

		// dispatcher
		float widthPixels = kernelWidth * g_screenWidth;
		if (widthPixels > KERNEL_SIZE_5_THRESHOLD) {
			return PS_Horizontal_Core_5(color, kernelWidth, input.texCoord);
		} else if (widthPixels > KERNEL_SIZE_3_THRESHOLD) {
			return PS_Horizontal_Core_3(color, kernelWidth, input.texCoord);
		} else {
			return color;
		}
	}
}

float4 PS_Vertical_3(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthY = diffuseStencil.y;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthY;

		// dispatcher
		float widthPixels = kernelWidth * g_screenHeight;
		if (widthPixels > KERNEL_SIZE_3_THRESHOLD) {
			return PS_Vertical_Core_3(color, kernelWidth, input.texCoord);
		} else {
			return color;
		}
	}
}

float4 PS_Horizontal_3(PS_INPUT input) : SV_Target {
	float4 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0);
	float stencil = color.a;

	if (stencil == 0.0) {
		return color;
	} else {
		float4 diffuseStencil = g_diffuseStencilTexture.SampleLevel(g_samPoint, input.texCoord, 0);
		float widthX = diffuseStencil.x;

		// kernel width for each channel
		float kernelWidth = g_blurWidth * widthX * g_invAspectRatio;

		// dispatcher
		float widthPixels = kernelWidth * g_screenWidth;
		if (widthPixels > KERNEL_SIZE_3_THRESHOLD) {
			return PS_Horizontal_Core_3(color, kernelWidth, input.texCoord);
		} else {
			return color;
		}
	}
}
