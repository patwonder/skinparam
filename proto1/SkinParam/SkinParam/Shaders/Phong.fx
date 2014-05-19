
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

// Implements the phong shading model

#include "Lighting.fx"
#include "Bump.fx"

struct VS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 worldPos : WORLDPOS;
	float2 texCoord : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output;
	float4 wpos4 = mul(input.pos, g_matWorld);
	output.normal = normalize(mul(float4(input.normal, 0.0), g_matWorld).xyz);
	output.tangent = normalize(mul(float4(input.tangent, 0.0), g_matWorld).xyz);
	output.binormal = normalize(mul(float4(input.binormal, 0.0), g_matWorld).xyz);

	float bumpAmount = g_material.bump_multiplier * 2 * (g_bump.SampleLevel(g_samBump, input.texCoord, 0).r - 0.5);
	wpos4.xyz += wpos4.w * bumpAmount * output.normal;

	output.worldPos = wpos4.xyz / wpos4.w;
	output.pos = mul(wpos4, g_matViewProj);
	output.color = input.color;
	output.texCoord = input.texCoord;
	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target {
	// texture
	float3 tex = g_texture.Sample(g_samTexture, input.texCoord).rgb;

	// calculated pertubated normal
#if USE_NORMAL_MAP == 0
	float3 vNormalWS = calcNormalWS(g_samBump, g_bump, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#else
	float3 vNormalWS = calcNormalFromNormalMap(g_samNormal, g_normalMap, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#endif
	
	// shading
	float3 color = phong_lighting(g_material, g_ambient, g_lights, g_posEye, input.worldPos, tex * input.color.rgb, normal);
	return float4(color, 1.0);
}
