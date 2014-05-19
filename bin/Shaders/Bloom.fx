// Bloom filter for specular light

#include "Quad.fx"

Texture2D g_screenTexture : register(t0);

cbuffer Bloom : register(b0) {
	float2 rcpScreenSize;
	uint sampleLevel; // only for vertical convolution
};

// Standard Deviation = 3.0
//static const int KERNEL_WIDTH = 21;
//static const float KERNEL[] = {0.0003, 0.0005, 0.0015, 0.0039, 0.0089, 0.0182, 0.0334, 0.0549, \
//0.0807, 0.1062, 0.1253, 0.1324, 0.1253, 0.1062, 0.0807, 0.0549, \
//0.0334, 0.0182, 0.0089, 0.0039, 0.0015, 0.0005, 0.0003};

// Standard Deviation ~ 0.9 (slightly smaller)
static const int KERNEL_WIDTH = 5;
static const float KERNEL[] = { 0.0478, 0.2415, 0.4214, 0.2415, 0.0478 };
//static const float KERNEL[] = {  0.2, 0.2, 0.2, 0.2, 0.2 };

static const int KERNEL_START = -(KERNEL_WIDTH - 1) / 2;
static const int KERNEL_END = -KERNEL_START;

static const float2 DETECT_DIR[] = { { 0.0, 0.0 }, { -1.0, 0.0 }, { 1.0, 0.0 }, { 0.0, -1.0 }, { 0.0, 1.0 } };


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

float4 PS_Detect(PS_INPUT input) : SV_Target {
	float3 color = 1e30;
	for (uint i = 0; i < 5; i++) {
		color = min(color, g_screenTexture.SampleLevel(g_samPoint, input.texCoord + DETECT_DIR[i] * rcpScreenSize, 0).rgb);
	}
	return float4(max(color - 1.0, 0.0), 1.0);
}

float4 PS_Vertical(PS_INPUT input) : SV_Target {
	float x = input.texCoord.x;
	float3 result = 0.0;
	for (int i = KERNEL_START; i <= KERNEL_END; i++) {
		float y = input.texCoord.y + i * rcpScreenSize.y;
		float3 sample = g_screenTexture.SampleLevel(g_samPoint, float2(x, y), sampleLevel).rgb;
		result += KERNEL[i - KERNEL_START] * sample;
	}
	return float4(result, 1.0);
}

// NOTE: the in-symmetricity is not a mistake
// First-pass: grab luminance > 1.0 pixels and convolve them vertically
// Second-pass : convolve the previous result horizontally
float4 PS_Horizontal(PS_INPUT input) : SV_Target {
	float y = input.texCoord.y;
	float3 result = 0.0;
	for (int i = KERNEL_START; i <= KERNEL_END; i++) {
		float x = input.texCoord.x + i * rcpScreenSize.x;
		result += KERNEL[i - KERNEL_START] * g_screenTexture.SampleLevel(g_samPoint, float2(x, y), 0).rgb;
	}
	return float4(result, 1.0);
}
