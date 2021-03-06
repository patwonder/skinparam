
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
