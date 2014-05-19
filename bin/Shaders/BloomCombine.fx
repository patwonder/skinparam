
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

// Combine bloom results

#include "Quad.fx"
#include "Lum.fx"

static const uint BLOOM_PASSES = 6;

Texture2D g_screenTexture : register(t0);
Texture2D g_bloom[BLOOM_PASSES] : register(t1);

float4 PS(PS_INPUT input) : SV_Target {
	float3 color = g_screenTexture.SampleLevel(g_samPoint, input.texCoord, 0).rgb;
	float3 finalColor = color;// + g_bloom[0].SampleLevel(g_samPoint, input.texCoord, 0).rgb;
	for (uint i = 1; i < BLOOM_PASSES; i++) {
		finalColor += g_bloom[i].SampleLevel(g_samLinear, input.texCoord, 0).rgb / (1 << i);
	}
	return float4(finalColor, 1.0);
}

float4 PS_AA(PS_INPUT input) : SV_Target {
	float3 color = PS(input).rgb;
	return lum_output_linear(color);
}
