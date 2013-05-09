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
