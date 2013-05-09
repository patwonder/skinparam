// Implements the phong shading model, with tesselation

#include "Lighting.fx"
#include "Culling.fx"
#include "Bump.fx"

cbuffer Tessellation : register(c3) {
	float4 g_vTessellationFactor; // Edge, inside, minimum tessellation factor and (half screen height/desired triangle size)
};

struct VS_INPUT {
	float4 vPosOS : POSITION;
	float4 color : COLOR;
	float3 vNormalOS : NORMAL;
	float3 vTangentOS : TANGENT;
	float3 vBinormalOS : BINORMAL;
	float2 texCoord : TEXCOORD0;
};

struct HS_INPUT {
	float3 vPosWS : WORLDPOS;
	float4 color : COLOR;
	float3 vNormalWS : NORMAL;
	float3 vTangentWS : TANGENT;
	float3 vBinormalWS : BINORMAL;
	float2 texCoord : TEXCOORD0;
};

struct HSCF_OUTPUT {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct DS_INPUT {
	float3 vPosWS : WORLDPOS;
	float4 color : COLOR;
	float3 vNormalWS : NORMAL;
	float3 vTangentWS : TANGENT;
	float3 vBinormalWS : BINORMAL;
	float2 texCoord : TEXCOORD0;
};

struct PS_INPUT {
	float4 vPosPS : SV_POSITION;
	float4 color : COLOR0;
	float3 vNormalWS : NORMAL;
	float3 vTangentWS : TANGENT;
	float3 vBinormalWS : BINORMAL;
	float3 vPosWS : WORLDPOS;
	float2 texCoord : TEXCOORD0;
};

// Vertex shader, do world transform only
HS_INPUT VS(VS_INPUT input) {
	HS_INPUT output;
	float4 wpos4 = mul(input.vPosOS, g_matWorld);
	output.vPosWS = wpos4.xyz / wpos4.w;
	output.color = input.color;
	output.vNormalWS = mul(input.vNormalOS, (float3x3)g_matWorld);
	output.vTangentWS = mul(input.vTangentOS, (float3x3)g_matWorld);
	output.vBinormalWS = mul(input.vBinormalOS, (float3x3)g_matWorld);
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
//[maxtessfactor(5.0)]
DS_INPUT HS(InputPatch<HS_INPUT, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
	DS_INPUT output;
	output.vPosWS = patch[pointId].vPosWS;
	output.color = patch[pointId].color;
	output.vNormalWS = patch[pointId].vNormalWS;
	output.vTangentWS = patch[pointId].vTangentWS;
	output.vBinormalWS = patch[pointId].vBinormalWS;
	output.texCoord = patch[pointId].texCoord;
	return output;
}

// Domain shader
[domain("tri")]
PS_INPUT DS(HSCF_OUTPUT tes, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DS_INPUT, 3> patch) {
	// interpolate new point data
	DS_INPUT input;
	input.vPosWS = uvwCoord.x * patch[0].vPosWS + uvwCoord.y * patch[1].vPosWS + uvwCoord.z * patch[2].vPosWS;
	input.color = uvwCoord.x * patch[0].color + uvwCoord.y * patch[1].color + uvwCoord.z * patch[2].color;
	input.vNormalWS = uvwCoord.x * patch[0].vNormalWS + uvwCoord.y * patch[1].vNormalWS + uvwCoord.z * patch[2].vNormalWS;
	input.vBinormalWS = uvwCoord.x * patch[0].vBinormalWS + uvwCoord.y * patch[1].vBinormalWS + uvwCoord.z * patch[2].vBinormalWS;
	input.vTangentWS = uvwCoord.x * patch[0].vTangentWS + uvwCoord.y * patch[1].vTangentWS + uvwCoord.z * patch[2].vTangentWS;
	input.texCoord = uvwCoord.x * patch[0].texCoord + uvwCoord.y * patch[1].texCoord + uvwCoord.z * patch[2].texCoord;

	// do view projection transform
	input.vNormalWS = normalize(input.vNormalWS);
	input.vTangentWS = normalize(input.vTangentWS);
	input.vBinormalWS = normalize(input.vBinormalWS);
	float bumpAmount = g_material.bump_multiplier * 2 * (g_bump.SampleLevel(g_samBump, input.texCoord, 0).r - 0.5);
	input.vPosWS += bumpAmount * input.vNormalWS;

	PS_INPUT output;
	output.vPosWS = input.vPosWS;
	output.vPosPS = mul(float4(input.vPosWS, 1.0), g_matViewProj);
	output.color = input.color;
	output.vNormalWS = input.vNormalWS;
	output.vTangentWS = input.vTangentWS;
	output.vBinormalWS = input.vBinormalWS;
	output.texCoord = input.texCoord;
	return output;
}

// Pixel shader
float4 PS(PS_INPUT input) : SV_Target {
	// textured color
	float3 tex_color = input.color.rgb * g_texture.Sample(g_samTexture, input.texCoord).rgb;

	// calculated pertubated normal
	float3 vNormalWS = calcNormalWS(g_samBump, g_bump, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
	
	// shading
	float3 color = phong_shadow(g_material, g_ambient, g_lights, g_posEye, input.vPosWS, tex_color, vNormalWS,
								g_shadowMaps, g_samShadow, g_matViewProjLights);
	return float4(color, 1.0);
}

// Irradiance domain & pixel shader
struct PS_INPUT_IR_DS {
	float4 vPosPS : SV_POSITION;
	float4 color : COLOR0;
	float3 vNormalWS : NORMAL;
	float3 vTangentWS : TANGENT;
	float3 vBinormalWS : BINORMAL;
	float3 vPosWS : WORLDPOS;
	float2 texCoord : TEXCOORD0;
	float depth : DEPTH;
};

struct PS_OUTPUT_IR_DS {
	// r, g, b: irradiance color; a: specular color
	float4 irradiance : SV_Target0;
	// albeto map, sqrt of tex_color * fresnel outgoing factor
	float4 albedo : SV_Target1;
	// r, g : diffuse NdotV in x, y direction; b : depth; a: stencil
	float4 diffuseStencil : SV_Target2;
	// specular light
	float4 specular : SV_Target3;
};

// Domain shader
[domain("tri")]
PS_INPUT_IR_DS DS_Irradiance(HSCF_OUTPUT tes, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DS_INPUT, 3> patch) {
	// interpolate new point data
	DS_INPUT input;
	input.vPosWS = uvwCoord.x * patch[0].vPosWS + uvwCoord.y * patch[1].vPosWS + uvwCoord.z * patch[2].vPosWS;
	input.color = uvwCoord.x * patch[0].color + uvwCoord.y * patch[1].color + uvwCoord.z * patch[2].color;
	input.vNormalWS = uvwCoord.x * patch[0].vNormalWS + uvwCoord.y * patch[1].vNormalWS + uvwCoord.z * patch[2].vNormalWS;
	input.vBinormalWS = uvwCoord.x * patch[0].vBinormalWS + uvwCoord.y * patch[1].vBinormalWS + uvwCoord.z * patch[2].vBinormalWS;
	input.vTangentWS = uvwCoord.x * patch[0].vTangentWS + uvwCoord.y * patch[1].vTangentWS + uvwCoord.z * patch[2].vTangentWS;
	input.texCoord = uvwCoord.x * patch[0].texCoord + uvwCoord.y * patch[1].texCoord + uvwCoord.z * patch[2].texCoord;

	// do view projection transform
	input.vNormalWS = normalize(input.vNormalWS);
	input.vTangentWS = normalize(input.vTangentWS);
	input.vBinormalWS = normalize(input.vBinormalWS);
	float bumpAmount = g_material.bump_multiplier * 2 * (g_bump.SampleLevel(g_samBump, input.texCoord, 0).r - 0.5);
	input.vPosWS += bumpAmount * input.vNormalWS;

	float4 vPosVS = mul(float4(input.vPosWS, 1.0), g_matView);

	PS_INPUT_IR_DS output;
	output.vPosWS = input.vPosWS;
	output.vPosPS = mul(float4(input.vPosWS, 1.0), g_matViewProj);
	output.color = input.color;
	output.vNormalWS = input.vNormalWS;
	output.vTangentWS = input.vTangentWS;
	output.vBinormalWS = input.vBinormalWS;
	output.texCoord = input.texCoord;
	output.depth = (vPosVS.z / vPosVS.w - 0.1) / 29.9;
	return output;
}

// Fdt / 2pi, which equals to 1 - Fdr/2pi
// using emprical equation from Jensen et al. 2001
static const float FRESNEL_REF_TOTAL = (-1.440 / (INDEX * INDEX) + 0.710 / INDEX + 0.668 + 0.0636 * INDEX) / (2 * PI);
static const float FRESNEL_TRANS_TOTAL = 1 - FRESNEL_REF_TOTAL;

PS_OUTPUT_IR_DS PS_Irradiance(PS_INPUT_IR_DS input) {
	PS_OUTPUT_IR_DS output;
	// textured irradiance color
	float3 tex_color = input.color.rgb * g_texture.Sample(g_samTexture, input.texCoord).rgb;
	float3 sq_tex_color = sqrt(tex_color);

	// calculated pertubated normal
	float3 vNormalWS = calcNormalWS(g_samBump, g_bump, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);

	// shading
	// global ambient
	float3 irradiance = g_ambient * g_material.ambient;
	float3 specular = 0.0;

	float3 N = normalize(vNormalWS);
	float3 V = normalize(g_posEye - input.vPosWS);
	// lights
	for (uint i = 0; i < NUM_LIGHTS; i++) {
		Light l = g_lights[i];

		// light vector & attenuation
		float3 L = normalize(l.position - input.vPosWS);
		float NdotL = dot(N, L);
		float atten = light_attenuation(input.vPosWS, l);
		// ambient
		float3 ambient = l.ambient * g_material.ambient;
		// diffuse
		float diffuseLight = max(NdotL, 0);
		float3 diffuse = atten * l.diffuse * g_material.diffuse * diffuseLight;
		// specular
		float3 H = normalize(L + V);
		float specularLight = saturate(CookTorrance(N, V, L, H, RMS_SLOPE));
		// look up shadow map for light amount
		float lightAmount = light_amount(input.vPosWS, g_shadowMaps[i], g_samShadow, g_matViewProjLights[i]);
		// fresnel transmittance for diffuse irradiance
		float fresnel_trans = saturate(1 - fresnel_term(NdotL));

		// putting them together
		specular += atten * l.specular * g_material.specular * specularLight * lightAmount;
		irradiance += ambient * FRESNEL_TRANS_TOTAL + diffuse * lightAmount * fresnel_trans;
	}

	// calculate fresnel outgoing factor
	float NdotV = dot(N, V);
	float fresnel_trans_view = 1;saturate(1 - fresnel_term(NdotV));

	// calculate kernel size reduction due to tilt surface
	float3 vNormalVS = normalize(mul(input.vNormalWS, (float3x3)g_matView)); // transform to camera view space
	// diffuse reduction equals to shadow length on corresponding plane
	float drx = sqrt(vNormalVS.y * vNormalVS.y + vNormalVS.z * vNormalVS.z);
	float dry = sqrt(vNormalVS.x * vNormalVS.x + vNormalVS.z * vNormalVS.z);

	output.irradiance = float4(sq_tex_color * irradiance, 1.0);
	output.albedo = float4(sq_tex_color * fresnel_trans_view, 1.0);
	output.diffuseStencil = float4(drx, dry, input.depth, 1.0);
	output.specular = float4(specular, 1.0);

	return output;
}
