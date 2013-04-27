// Implements the phong shading model, with tesselation

#include "Lighting.fx"

struct VS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct HS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct HSCF_OUTPUT {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct DS_INPUT {
	float4 pos : POSITION;
	float4 color : COLOR;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct PS_INPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : TEXCOORD0;
	float3 worldPos : TEXCOORD1;
	float2 texCoord : TEXCOORD2;
};

// Vertex shader : pass everything along
HS_INPUT VS(VS_INPUT input) {
	HS_INPUT output;
	output.pos = input.pos;
	output.color = input.color;
	output.normal = input.normal;
	output.texCoord = input.texCoord;
	return output;
}

// Hull shader patch constant function
HSCF_OUTPUT HSCF(InputPatch<HS_INPUT, 3> patch, uint patchId : SV_PrimitiveID) {
    HSCF_OUTPUT output;

    // Set the tessellation factors for the three edges of the triangle.
	float tessellationAmount = 2.0;
    output.edges[0] = tessellationAmount;
    output.edges[1] = tessellationAmount;
    output.edges[2] = tessellationAmount;

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside = tessellationAmount;

    return output;
}

// Hull shader
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSCF")]
DS_INPUT HS(InputPatch<HS_INPUT, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
	DS_INPUT output;
	output.pos = patch[pointId].pos;
	output.color = patch[pointId].color;
	output.normal = patch[pointId].normal;
	output.texCoord = patch[pointId].texCoord;
	return output;
}

// Domain shader
[domain("tri")]
PS_INPUT DS(HSCF_OUTPUT tes, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DS_INPUT, 3> patch) {
	// interpolate new point data
	DS_INPUT input;
	input.pos = uvwCoord.x * patch[0].pos + uvwCoord.y * patch[1].pos + uvwCoord.z * patch[2].pos;
	input.color = uvwCoord.x * patch[0].color + uvwCoord.y * patch[1].color + uvwCoord.z * patch[2].color;
	input.normal = uvwCoord.x * patch[0].normal + uvwCoord.y * patch[1].normal + uvwCoord.z * patch[2].normal;
	input.texCoord = uvwCoord.x * patch[0].texCoord + uvwCoord.y * patch[1].texCoord + uvwCoord.z * patch[2].texCoord;

	// do what should be typically done in the vertex shader
	PS_INPUT output;
	float4 wpos4 = mul(input.pos, g_matWorld);
	output.worldPos = wpos4.xyz / wpos4.w;
	output.pos = mul(wpos4, g_matViewProj);
	output.color = input.color;
	output.normal = mul(input.normal, (float3x3)g_matWorld);
	output.texCoord = input.texCoord;
	return output;
}

// Pixel shader
float4 PS(PS_INPUT input) : SV_Target {
	// textured color
	float3 tex_color = input.color.rgb * g_texture.Sample(g_samTexture, input.texCoord).rgb;
	float3 color = phong_lighting(g_material, g_ambient, g_lights, g_posEye, input.worldPos, tex_color, input.normal);
	return float4(color, 1.0);
}
