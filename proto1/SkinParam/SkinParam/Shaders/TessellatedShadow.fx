// Shadow map renderer

#include "Lighting.fx"
#include "Culling.fx"

cbuffer Tessellation : register(c3) {
	float4 g_vTessellationFactor; // Edge, inside, minimum tessellation factor and (half screen height/desired triangle size)
};

struct VS_INPUT {
	float4 vPosOS : POSITION;
	float3 vNormalOS : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct HS_INPUT {
	float3 vPosWS : WORLDPOS;
	float3 vNormalWS : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct HSCF_OUTPUT {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct DS_INPUT {
	float3 vPosWS : WORLDPOS;
	float3 vNormalWS : NORMAL;
	float2 texCoord : TEXCOORD0;
};

struct PS_INPUT {
	float4 vPosPS : SV_POSITION;
	float2 depth : TEXCOORD0;
};

// Vertex shader, do world transform only
HS_INPUT VS(VS_INPUT input) {
	HS_INPUT output;
	float4 wpos4 = mul(input.vPosOS, g_matWorld);
	output.vPosWS = wpos4.xyz / wpos4.w;
	output.vNormalWS = mul(input.vNormalOS, (float3x3)g_matWorld);
	output.texCoord = input.texCoord;
	return output;
}

// Hull shader patch constant function
HSCF_OUTPUT HSCF(InputPatch<HS_INPUT, 3> patch, uint patchId : SV_PrimitiveID) {
    HSCF_OUTPUT output;

	// Calcuate screen space positions for each vertex
	float2 vPosPS0 = getScreenSpacePosition(patch[0].vPosWS, g_matViewProj);
	float2 vPosPS1 = getScreenSpacePosition(patch[1].vPosWS, g_matViewProj);
	float2 vPosPS2 = getScreenSpacePosition(patch[2].vPosWS, g_matViewProj);

    // Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = max(g_vTessellationFactor.z, g_vTessellationFactor.w * distance(vPosPS2, vPosPS1));
    output.edges[1] = max(g_vTessellationFactor.z, g_vTessellationFactor.w * distance(vPosPS0, vPosPS2));
    output.edges[2] = max(g_vTessellationFactor.z, g_vTessellationFactor.w * distance(vPosPS0, vPosPS1));

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside = 0.333 * (output.edges[0] + output.edges[1] + output.edges[2]);

    // View frustum culling
    bool bViewFrustumCull = ViewFrustumCull(patch[0].vPosWS, patch[1].vPosWS, patch[2].vPosWS,
											g_vFrustumPlaneEquation, g_material.bump_multiplier);
    if (bViewFrustumCull) {
        // Set all tessellation factors to 0 if frustum cull test succeeds
        output.edges[0] = 0.0;
        output.edges[1] = 0.0;
        output.edges[2] = 0.0;
        output.inside   = 0.0;
    }

    return output;
}

// Hull shader
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSCF")]
[maxtessfactor(20.0)]
DS_INPUT HS(InputPatch<HS_INPUT, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
	DS_INPUT output;
	output.vPosWS = patch[pointId].vPosWS;
	output.vNormalWS = patch[pointId].vNormalWS;
	output.texCoord = patch[pointId].texCoord;
	return output;
}

// Domain shader
[domain("tri")]
PS_INPUT DS(HSCF_OUTPUT tes, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DS_INPUT, 3> patch) {
	// interpolate new point data
	DS_INPUT input;
	input.vPosWS = uvwCoord.x * patch[0].vPosWS + uvwCoord.y * patch[1].vPosWS + uvwCoord.z * patch[2].vPosWS;
	input.vNormalWS = uvwCoord.x * patch[0].vNormalWS + uvwCoord.y * patch[1].vNormalWS + uvwCoord.z * patch[2].vNormalWS;
	input.texCoord = uvwCoord.x * patch[0].texCoord + uvwCoord.y * patch[1].texCoord + uvwCoord.z * patch[2].texCoord;

	// do view projection transform
	input.vNormalWS = normalize(input.vNormalWS);
	float bumpAmount = g_material.bump_multiplier * 2 * (g_bump.SampleLevel(g_samBump, input.texCoord, 0).r - 0.5);
	input.vPosWS += bumpAmount * input.vNormalWS;

	PS_INPUT output;
	output.vPosPS = mul(float4(input.vPosWS, 1.0), g_matViewProj);
	output.depth = output.vPosPS.zw;
	return output;
}

// Pixel shader
float4 PS(PS_INPUT input) : SV_Target {
	// output depth
	return input.depth.x / input.depth.y;
}