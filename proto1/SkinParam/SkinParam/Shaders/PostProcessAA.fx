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
	return FxaaPixelShader(pos, posPos, tex, rcpFrame, rcpFrameOpt);
}
