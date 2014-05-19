
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

// Post process anti-aliasing

#include "Quad.fx"

// FXAA #define s
#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_LINEAR 1

#include "FXAA.fx"

cbuffer PostProcess : register(b0) {
  // {x_} = 1.0/screenWidthInPixels
  // {_y} = 1.0/screenHeightInPixels
  float2 rcpFrame;

  // This must be from a constant/uniform.
  // {x___} = 2.0/screenWidthInPixels
  // {_y__} = 2.0/screenHeightInPixels
  // {__z_} = 0.5/screenWidthInPixels
  // {___w} = 0.5/screenHeightInPixels
  float4 rcpFrameOpt;
};

Texture2D g_frame : register(t0);

float4 PS(PS_INPUT input) : SV_Target {
	float2 pos = input.texCoord;
	float4 posPos = float4(pos - rcpFrameOpt.zw, pos + rcpFrameOpt.zw);

	FxaaTex tex;
	tex.smpl = g_samLinear;
	tex.tex = g_frame;
	return float4(FxaaPixelShader(pos, posPos, tex, rcpFrame, rcpFrameOpt).rgb, 1.0);
}
