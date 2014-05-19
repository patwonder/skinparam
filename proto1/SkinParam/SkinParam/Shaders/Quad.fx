
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

// Screen quad auto generation without the need of a vertex buffer

SamplerState g_samLinear : register(s0);
SamplerState g_samPoint : register(s1);

struct PS_INPUT {
	float4 vPosPS : SV_Position;
	float2 texCoord : TEXCOORD0;
};

// No vs buffer, generate the quad on the fly
// Assume topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
static const float4 SCREEN_QUAD[] = { { -1.0, -1.0, 0.0, 1.0 }, { -1.0, 1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 },
									  { -1.0, -1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 }, { 1.0, -1.0, 0.0, 1.0 } };
static const float2 SCREEN_TEX[] = { { 0.0, 1.0 }, { 0.0, 0.0 }, { 1.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 0.0 }, { 1.0, 1.0 } };

PS_INPUT VS_Quad(uint vertexId : SV_VertexID) {
	PS_INPUT output;
	output.vPosPS = SCREEN_QUAD[vertexId];
	output.texCoord = SCREEN_TEX[vertexId];
	return output;
}
