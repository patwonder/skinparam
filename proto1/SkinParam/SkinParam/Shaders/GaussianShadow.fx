
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
