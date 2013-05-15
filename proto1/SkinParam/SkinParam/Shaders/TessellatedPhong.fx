// Implements the phong shading model, with tesselation

#include "Lighting.fx"
#include "Culling.fx"
#include "Bump.fx"

cbuffer Tessellation : register(b3) {
	float4 g_vTessellationFactor; // Edge, inside, minimum tessellation factor and (half screen height/desired triangle size)
};

cbuffer SSS : register(b4) {
	float g_sss_intensity;
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

// Vertex shader without tessellation
PS_INPUT VS_NoTessellation(VS_INPUT input) {
	float4 wpos4 = mul(input.vPosOS, g_matWorld);
	float3 vPosWS = wpos4.xyz / wpos4.w;
	float3 vNormalWS = mul(input.vNormalOS, (float3x3)g_matWorld);
	float3 vTangentWS = mul(input.vTangentOS, (float3x3)g_matWorld);
	float3 vBinormalWS = mul(input.vBinormalOS, (float3x3)g_matWorld);

	// do view projection transform
	vNormalWS = normalize(vNormalWS);
	float bumpAmount = g_material.bump_multiplier * 2 * (g_bump.SampleLevel(g_samBump, input.texCoord, 0).r - 0.5);
	vPosWS += bumpAmount * vNormalWS;

	PS_INPUT output;
	output.vPosWS = vPosWS;
	output.vPosPS = mul(float4(vPosWS, 1.0), g_matViewProj);
	output.color = input.color;
	output.vNormalWS = vNormalWS;
	output.vTangentWS = vTangentWS;
	output.vBinormalWS = vBinormalWS;
	output.texCoord = input.texCoord;
	return output;
}

// Hull shader patch constant function
HSCF_OUTPUT HSCF(InputPatch<HS_INPUT, 3> patch, uint patchId : SV_PrimitiveID) {
    HSCF_OUTPUT output;

	// View frustum culling
	bool bViewFrustumCull = ViewFrustumCull(patch[0].vPosWS, patch[1].vPosWS, patch[2].vPosWS,
											g_vFrustumPlaneEquation, g_material.bump_multiplier);
	if (bViewFrustumCull) {
		// Set all tessellation factors to 0 if frustum cull test succeeds
		output.edges[0] = 0.0;
		output.edges[1] = 0.0;
		output.edges[2] = 0.0;
		output.inside   = 0.0;
	} else {
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
#if USE_NORMAL_MAP == 0
	float3 vNormalWS = calcNormalWS(g_samBump, g_bump, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#else
	float3 vNormalWS = calcNormalFromNormalMap(g_samNormal, g_normalMap, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#endif
	
	// shading
	float3 color = phong_shadow(g_material, g_ambient, g_lights, g_posEye, input.vPosWS, tex_color, vNormalWS,
								g_shadowMaps, g_samShadow, g_matViewLights, g_matViewProjLights);
	return float4(color, 1.0);
}

// Irradiance domain & pixel shader
struct PS_INPUT_IRRADIANCE {
	float4 vPosPS : SV_POSITION;
	float4 color : COLOR0;
	float3 vNormalWS : NORMAL;
	float3 vTangentWS : TANGENT;
	float3 vBinormalWS : BINORMAL;
	float3 vPosWS : WORLDPOS;
	float2 texCoord : TEXCOORD0;
	float depth : DEPTH;
};

struct PS_OUTPUT_IRRADIANCE {
	// r, g, b: irradiance color; a: stencil
	float4 irradiance : SV_Target0;
	// albeto map, sqrt of tex_color * fresnel outgoing factor
	float4 albedo : SV_Target1;
	// r, g : kernel size in x, y direction; b : depth; a: stencil
	float4 diffuseStencil : SV_Target2;
	// specular light
	float4 specular : SV_Target3;
};

PS_INPUT_IRRADIANCE VS_Irradiance_NoTessellation(VS_INPUT input) {
	float4 wpos4 = mul(input.vPosOS, g_matWorld);
	float3 vPosWS = wpos4.xyz / wpos4.w;
	float3 vNormalWS = mul(input.vNormalOS, (float3x3)g_matWorld);
	float3 vTangentWS = mul(input.vTangentOS, (float3x3)g_matWorld);
	float3 vBinormalWS = mul(input.vBinormalOS, (float3x3)g_matWorld);

	// do view projection transform
	vNormalWS = normalize(vNormalWS);
	float bumpAmount = g_material.bump_multiplier * 2 * (g_bump.SampleLevel(g_samBump, input.texCoord, 0).r - 0.5);
	vPosWS += bumpAmount * vNormalWS;

	float4 vPosVS = mul(float4(vPosWS, 1.0), g_matView);

	PS_INPUT_IRRADIANCE output;
	output.vPosWS = vPosWS;
	output.vPosPS = mul(float4(vPosWS, 1.0), g_matViewProj);
	output.color = input.color;
	output.vNormalWS = vNormalWS;
	output.vTangentWS = vTangentWS;
	output.vBinormalWS = vBinormalWS;
	output.texCoord = input.texCoord;
	output.depth = vPosVS.z / vPosVS.w;
	return output;
}

// Domain shader
[domain("tri")]
PS_INPUT_IRRADIANCE DS_Irradiance(HSCF_OUTPUT tes, float3 uvwCoord : SV_DomainLocation, const OutputPatch<DS_INPUT, 3> patch) {
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

	PS_INPUT_IRRADIANCE output;
	output.vPosWS = input.vPosWS;
	output.vPosPS = mul(float4(input.vPosWS, 1.0), g_matViewProj);
	output.color = input.color;
	output.vNormalWS = input.vNormalWS;
	output.vTangentWS = input.vTangentWS;
	output.vBinormalWS = input.vBinormalWS;
	output.texCoord = input.texCoord;
	output.depth = vPosVS.z / vPosVS.w;
	return output;
}

// kernel width calculation
// one unit length in world space equals 120mm
static const float MM_PER_LENGTH = 120;
static const float FOV_ANGLE = 30;
// calculate depth * (length per mm in texture space)
static const float SIZE_ALPHA = 1 / (2 * tan((FOV_ANGLE / 180 * PI) / 2) * MM_PER_LENGTH);

float getKernelSize(float depthVS, float grad) {
	return SIZE_ALPHA / depthVS * grad;
}

static const float DD_START = 0.01;
static const float DD_LEN = 0.01;

float getDDXDepthPenalty(float depth) {
	return saturate(1 - (abs(ddx(depth)) - DD_START) / DD_LEN);
}

float getDDYDepthPenalty(float depth) {
	return saturate(1 - (abs(ddy(depth)) - DD_START) / DD_LEN);
}

// Fdt / 2pi, which equals to 1 - Fdr/2pi
// using emprical equation from Jensen et al. 2001
static const float FRESNEL_REF_TOTAL = (-1.440 / (INDEX * INDEX) + 0.710 / INDEX + 0.668 + 0.0636 * INDEX) / (2 * PI);
static const float FRESNEL_TRANS_TOTAL = 1 - FRESNEL_REF_TOTAL;

// calculate transmittance attenuated by normalized distance s
float3 trans_atten(float s) {
	float ns2 = -s * s;
	return float3(0.233, 0.455, 0.649) * exp(ns2 / 0.0064)
		 + float3(0.100, 0.336, 0.344) * exp(ns2 / 0.0484)
		 + float3(0.118, 0.198, 0.000) * exp(ns2 / 0.1870)
		 + float3(0.113, 0.007, 0.007) * exp(ns2 / 0.5670)
		 + float3(0.358, 0.004, 0.000) * exp(ns2 / 1.9900)
		 + float3(0.078, 0.000, 0.000) * exp(ns2 / 7.4100);
}

float distance(float3 vPosWS, float3 vNormalWS, float ldepth, matrix matViewProjLight,
			   Texture2D shadowMap, SamplerState samShadow)
{
	float3 vShrinkedPosWS = vPosWS;
	float4 vPosPSL4 = mul(float4(vShrinkedPosWS, 1.0), matViewProjLight);
	float2 vPosTeSL = float2(0.5, -0.5) * (vPosPSL4.xy / vPosPSL4.w) + 0.5;
	float2 sample = shadowMap.Sample(samShadow, vPosTeSL).rg;
	// take the limit of 84.13% confidence interval (one way) as our depth estimation
	// prevent artifacts at glancing angles of light
	float variance = max(sample.g - sample.r * sample.r, 0.0);
	float sd = sqrt(variance);
	float d1f = sample.r + sd;
	float d1n = sample.r - sd;
	float d2 = ldepth;
	return max(d2 - d1n, sd);
}

float3 backlit_amount(float3 vPosWS, float3 vNormalWS, float3 L, float ldepth, matrix matViewProjLight,
					  Texture2D shadowMap, SamplerState samShadow) {
	float s = 0.25 * MM_PER_LENGTH * distance(vPosWS, vNormalWS, ldepth, matViewProjLight, shadowMap, samShadow);
	float3 atten = trans_atten(s);
	float NdotL = max(0.3 + dot(-vNormalWS, L), 0);
	return NdotL * atten;
}

PS_OUTPUT_IRRADIANCE PS_Irradiance(PS_INPUT_IRRADIANCE input) {
	PS_OUTPUT_IRRADIANCE output;
	// textured irradiance color
	float3 tex_color = input.color.rgb * g_texture.Sample(g_samTexture, input.texCoord).rgb;
	float3 sq_tex_color = sqrt(tex_color);

	input.vNormalWS = normalize(input.vNormalWS);

	// calculated pertubated normal
#if USE_NORMAL_MAP == 0
	float3 vNormalWS = calcNormalWS(g_samBump, g_bump, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#else
	float3 vNormalWS = calcNormalFromNormalMap(g_samNormal, g_normalMap, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#endif

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
		float3 L = l.position - input.vPosWS;
		float ldepth = length(L);
		L /= ldepth;
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
		float lightAmount = light_amount(input.vPosWS, g_shadowMaps[i], g_samShadow, ldepth, g_matViewProjLights[i]);
		// fresnel transmittance for diffuse irradiance
		float fresnel_trans = 1;

		// calculate transmittance from back of the object
		float3 backlitAmount = backlit_amount(input.vPosWS, input.vNormalWS, L, ldepth, g_matViewProjLights[i],
			g_shadowMaps[i], g_samShadow) * (1 - lightAmount) * g_sss_intensity;
		float3 backlit = atten * backlitAmount * l.diffuse * g_material.diffuse;

		// putting them together
		specular += atten * l.specular * g_material.specular * specularLight * lightAmount;
		irradiance += ambient * FRESNEL_TRANS_TOTAL + diffuse * lightAmount * fresnel_trans + backlit;
	}

	// calculate fresnel outgoing factor
	float fresnel_trans_view = 1;

	// calculate kernel size reduction due to tilt surface
	//  1 - calculate local orthogonal coordinate system
	float3 vUp = float3(g_matView[0].y, g_matView[1].y, g_matView[2].y);
	float3 vHorLocal = cross(V, vUp);
	float3 vVerLocal = cross(V, vHorLocal);
	//  2 - diffuse reduction equals to shadow length on corresponding plane
	vNormalWS = input.vNormalWS;
	float drx = length(vNormalWS - dot(vNormalWS, vHorLocal) * vHorLocal);
	float dry = length(vNormalWS - dot(vNormalWS, vVerLocal) * vVerLocal);
	//  3 - kernel size, with reduction applied
	float kSizeX = getKernelSize(input.depth, drx) * getDDXDepthPenalty(input.depth);
	float kSizeY = getKernelSize(input.depth, dry) * getDDYDepthPenalty(input.depth);

	output.irradiance = float4(sq_tex_color * irradiance, g_sss_intensity);
	output.albedo = float4(sq_tex_color * fresnel_trans_view, 1.0);
	output.diffuseStencil = float4(kSizeX, kSizeY, input.depth, g_sss_intensity);
	output.specular = float4(specular, 1.0);

	return output;
}
