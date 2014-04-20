// Implements the phong shading model, with tesselation

#include "Lighting.fx"
#include "Culling.fx"
#include "Bump.fx"
#include "Lum.fx"

cbuffer Tessellation : register(b3) {
	float4 g_vTessellationFactor; // Edge, inside, minimum tessellation factor and (half screen height/desired triangle size)
};

static const uint NUM_GAUSSIANS = 6;

cbuffer SSS : register(b4) {
	float4 g_sss_coeff_sigma2[NUM_GAUSSIANS];
	float4 g_sss_color_tone;
	float g_sss_intensity;
	float g_sss_strength;
};

struct VS_INPUT {
	float4 vPosOS : POSITION;
	float4 color : COLOR;
	float3 vNormalOS : NORMAL;
	float3 vTangentOS : TANGENT;
	float3 vBinormalOS : BINORMAL;
	float2 texCoord : TEXCOORD0;
	float2 bumpOverride : TEXCOORD1;
};

struct HS_DS_INPUT {
	float3 vPosWS : WORLDPOS;
	float4 color : COLOR;
	float3 vNormalWS : NORMAL;
	float3 vTangentWS : TANGENT;
	float3 vBinormalWS : BINORMAL;
	float2 texCoord : TEXCOORD0;
	float2 bumpOverride : BUMP;
};

struct HSCF_OUTPUT {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct PS_INPUT {
	float4 vPosPS : SV_POSITION;
	float4 color : COLOR;
	float3 vNormalWS : NORMAL;
	float3 vTangentWS : TANGENT;
	float3 vBinormalWS : BINORMAL;
	float3 vPosWS : WORLDPOS;
	float2 texCoord : TEXCOORD0;
};

// Vertex shader, do world transform only
HS_DS_INPUT VS_Core_Part_1(VS_INPUT input) {
	HS_DS_INPUT output;
	float4 wpos4 = mul(input.vPosOS, g_matWorld);
	output.vPosWS = wpos4.xyz / wpos4.w;
	output.color = input.color;
	output.vNormalWS = mul(input.vNormalOS, (float3x3)g_matWorld);
	output.vTangentWS = mul(input.vTangentOS, (float3x3)g_matWorld);
	output.vBinormalWS = mul(input.vBinormalOS, (float3x3)g_matWorld);
	output.texCoord = input.texCoord;
	output.bumpOverride = input.bumpOverride;
	return output;
}

PS_INPUT VS_Core_Part_2(HS_DS_INPUT input) {
	// do view projection transform
	input.vNormalWS = normalize(input.vNormalWS);
	float bumpSample = lerp(g_bump.SampleLevel(g_samBump, input.texCoord, 0).r, input.bumpOverride.x, input.bumpOverride.y);
	float bumpAmount = g_material.bump_multiplier * 2 * (bumpSample - 0.5);
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

HS_DS_INPUT VS(VS_INPUT input) {
	return VS_Core_Part_1(input);
}

// Vertex shader without tessellation
PS_INPUT VS_NoTessellation(VS_INPUT input) {
	return VS_Core_Part_2(VS_Core_Part_1(input));
}

// Hull shader patch constant function
HSCF_OUTPUT HSCF(InputPatch<HS_DS_INPUT, 3> patch, uint patchId : SV_PrimitiveID) {
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
HS_DS_INPUT HS(InputPatch<HS_DS_INPUT, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
	return patch[pointId];
}

// Domain shader
[domain("tri")]
PS_INPUT DS(HSCF_OUTPUT tes, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HS_DS_INPUT, 3> patch) {
	// interpolate new point data
	HS_DS_INPUT input;
	input.vPosWS = uvwCoord.x * patch[0].vPosWS + uvwCoord.y * patch[1].vPosWS + uvwCoord.z * patch[2].vPosWS;
	input.color = uvwCoord.x * patch[0].color + uvwCoord.y * patch[1].color + uvwCoord.z * patch[2].color;
	input.vNormalWS = uvwCoord.x * patch[0].vNormalWS + uvwCoord.y * patch[1].vNormalWS + uvwCoord.z * patch[2].vNormalWS;
	input.vBinormalWS = uvwCoord.x * patch[0].vBinormalWS + uvwCoord.y * patch[1].vBinormalWS + uvwCoord.z * patch[2].vBinormalWS;
	input.vTangentWS = uvwCoord.x * patch[0].vTangentWS + uvwCoord.y * patch[1].vTangentWS + uvwCoord.z * patch[2].vTangentWS;
	input.texCoord = uvwCoord.x * patch[0].texCoord + uvwCoord.y * patch[1].texCoord + uvwCoord.z * patch[2].texCoord;
	input.bumpOverride = uvwCoord.x * patch[0].bumpOverride + uvwCoord.y * patch[1].bumpOverride + uvwCoord.z * patch[2].bumpOverride;

	input.vTangentWS = normalize(input.vTangentWS);
	input.vBinormalWS = normalize(input.vBinormalWS);

	return VS_Core_Part_2(input);
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
	float4 color : COLOR;
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

PS_INPUT_IRRADIANCE VS_Irradiance_Core_Part_2(HS_DS_INPUT input) {
	// do view projection transform
	input.vNormalWS = normalize(input.vNormalWS);
	float bumpSample = lerp(g_bump.SampleLevel(g_samBump, input.texCoord, 0).r, input.bumpOverride.x, input.bumpOverride.y);
	float bumpAmount = g_material.bump_multiplier * 2 * (bumpSample - 0.5);
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

PS_INPUT_IRRADIANCE VS_Irradiance_NoTessellation(VS_INPUT input) {
	return VS_Irradiance_Core_Part_2(VS_Core_Part_1(input));
}

// Domain shader
[domain("tri")]
PS_INPUT_IRRADIANCE DS_Irradiance(HSCF_OUTPUT tes, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HS_DS_INPUT, 3> patch) {
	// interpolate new point data
	HS_DS_INPUT input;
	input.vPosWS = uvwCoord.x * patch[0].vPosWS + uvwCoord.y * patch[1].vPosWS + uvwCoord.z * patch[2].vPosWS;
	input.color = uvwCoord.x * patch[0].color + uvwCoord.y * patch[1].color + uvwCoord.z * patch[2].color;
	input.vNormalWS = uvwCoord.x * patch[0].vNormalWS + uvwCoord.y * patch[1].vNormalWS + uvwCoord.z * patch[2].vNormalWS;
	input.vBinormalWS = uvwCoord.x * patch[0].vBinormalWS + uvwCoord.y * patch[1].vBinormalWS + uvwCoord.z * patch[2].vBinormalWS;
	input.vTangentWS = uvwCoord.x * patch[0].vTangentWS + uvwCoord.y * patch[1].vTangentWS + uvwCoord.z * patch[2].vTangentWS;
	input.texCoord = uvwCoord.x * patch[0].texCoord + uvwCoord.y * patch[1].texCoord + uvwCoord.z * patch[2].texCoord;
	input.bumpOverride = uvwCoord.x * patch[0].bumpOverride + uvwCoord.y * patch[1].bumpOverride + uvwCoord.z * patch[2].bumpOverride;

	input.vTangentWS = normalize(input.vTangentWS);
	input.vBinormalWS = normalize(input.vBinormalWS);

	return VS_Irradiance_Core_Part_2(input);
}

// kernel width calculation
// one unit length in world space equals 80mm
static const float MM_PER_LENGTH = 80;
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

// Fdt, which equals to 1 - Fdr
// using emprical equation from Jensen et al. 2001
static const float FRESNEL_REF_TOTAL = -0.44 + 0.71 * INDEX - 0.332 * INDEX * INDEX + 0.0636 * INDEX * INDEX * INDEX;
static const float FRESNEL_TRANS_TOTAL = 1 - FRESNEL_REF_TOTAL;

/*
static const float3 FALLOFF_DATA[] = {
	{1., 1., 1.}, {0.917082, 0.992145, 0.989227}, {0.787153, 0.969239, 
	0.957843}, {0.714713, 0.933157, 0.908512}, {0.673245, 0.886719, 
	0.845236}, {0.637461, 0.833288, 0.772787}, {0.604141, 0.776339, 
	0.696086}, {0.573639, 0.719052, 0.619631}, {0.545713, 0.664014, 
	0.547067}, {0.519803, 0.613052, 0.480949}, {0.495422, 0.567201, 
	0.422703}, {0.472274, 0.526794, 0.372744}, {0.450215, 0.49161, 
	0.330703}, {0.429171, 0.461071, 0.295692}, {0.409081, 0.434415, 
	0.266566}, {0.389879, 0.410843, 0.242127}, {0.371489, 0.389625, 
	0.221276}, {0.353834, 0.370153, 0.203088}, {0.336847, 0.351962, 
	0.186849}, {0.320475, 0.334724, 0.17204}, {0.304678, 0.318226, 
	0.158311}, {0.289427, 0.302346, 0.145445}, {0.274707, 0.287023, 
	0.133315}, {0.260507, 0.272235, 0.121858}, {0.24682, 0.257986, 
	0.111047}, {0.233643, 0.244287, 0.100875}, {0.220973, 0.231156, 
	0.0913403}, {0.208807, 0.218604, 0.0824453}, {0.197139, 0.206641, 
	0.0741875}, {0.185966, 0.195271, 0.0665602}, {0.175281, 0.18449, 
	0.059551}, {0.165078, 0.17429, 0.0531424}, {0.155351, 0.164658, 
	0.0473122}, {0.14609, 0.155577, 0.0420341}, {0.137287, 0.147026, 
	0.0372791}, {0.128934, 0.138981, 0.0330154}, {0.121019, 0.131418, 
	0.0292101}, {0.113533, 0.12431, 0.0258291}, {0.106464, 0.117629, 
	0.0228384}, {0.0997994, 0.11135, 0.0202042}, {0.093526, 0.105445, 
	0.0178935}, {0.0876304, 0.0998895, 0.0158747}, {0.0820979, 
	0.0946575, 0.0141175}, {0.0769136, 0.0897258, 0.0125934}, {0.072062,
	0.0850726, 0.0112758}, {0.0675272, 0.0806771, 0.01014}, {0.0632933,
	0.0765205, 0.00916349}, {0.059344, 0.0725853, 
	0.00832572}, {0.0556632, 0.0688557, 0.00760814}, {0.052235, 
	0.0653174, 0.00699411}, {0.0490435, 0.0619574, 
	0.00646885}, {0.0460734, 0.0587642, 0.00601928}, {0.0433098, 
	0.0557273, 0.00563395}, {0.0407383, 0.0528375, 
	0.00530284}, {0.0383451, 0.0500864, 0.0050173}, {0.036117, 
	0.0474666, 0.00476988}, {0.0340415, 0.0449714, 
	0.00455418}, {0.0321069, 0.0425949, 0.0043648}, {0.0303021, 
	0.0403316, 0.00419715}, {0.0286168, 0.0381765, 
	0.00404738}, {0.0270413, 0.0361252, 0.00391227}, {0.0255669, 
	0.0341734, 0.00378916}, {0.0241852, 0.0323172, 
	0.00367585}, {0.0228887, 0.0305529, 0.00357054}, {0.0216706, 
	0.0288772, 0.00347175}, {0.0205246, 0.0272865, 
	0.00337829}, {0.0194448, 0.0257777, 0.00328921}, {0.0184261, 
	0.0243478, 0.00320374}, {0.0174638, 0.0229935, 
	0.00312127}, {0.0165537, 0.0217121, 0.00304132}, {0.0156918, 
	0.0205005, 0.00296352}, {0.0148749, 0.019356, 
	0.00288759}, {0.0140996, 0.0182757, 0.00281329}, {0.0133633, 
	0.0172569, 0.00274047}, {0.0126635, 0.0162968, 
	0.00266899}, {0.0119978, 0.0153929, 0.00259876}, {0.0113643, 
	0.0145424, 0.00252972}, {0.0107611, 0.013743, 
	0.00246182}, {0.0101865, 0.012992, 0.00239502}, {0.00963913, 
	0.012287, 0.00232932}, {0.00911752, 0.0116258, 
	0.00226469}, {0.00862046, 0.0110058, 0.00220113}, {0.00814678, 
	0.010425, 0.00213864}, {0.00769544, 0.00988117, 
	0.00207721}, {0.00726546, 0.0093722, 0.00201687}, {0.00685591, 
	0.00889608, 0.0019576}, {0.00646595, 0.00845088, 
	0.00189941}, {0.00609478, 0.00803473, 0.00184231}, {0.00574163, 
	0.00764586, 0.0017863}, {0.00540578, 0.00728256, 
	0.00173139}, {0.00508655, 0.00694319, 0.00167758}, {0.00478326, 
	0.00662622, 0.00162486}, {0.00449529, 0.00633017, 
	0.00157326}, {0.00422202, 0.00605365, 0.00152275}, {0.00396287, 
	0.00579532, 0.00147335}, {0.00371725, 0.00555395, 
	0.00142504}, {0.00348462, 0.00532834, 0.00137784}, {0.00326444, 
	0.00511739, 0.00133173}, {0.00305618, 0.00492005, 
	0.0012867}, {0.00285934, 0.00473534, 0.00124277}, {0.00267342, 
	0.00456234, 0.0011999},
	{0, 0, 0} // required
};
static const float3 FALLOFF_SAMPLE_DISTANCE = { 0.05, 0.01, 0.01 };
static const float FALLOFF_SAMPLE_COUNT = 100;
*/
// calculate transmittance attenuated by normalized distance s
float3 trans_atten(float s) {
	/*
	// linear sampling of FALLOFF_DATA
	float3 samplePoint = clamp(float3(s, s, s) / FALLOFF_SAMPLE_DISTANCE, 0, FALLOFF_SAMPLE_COUNT);
	float3 lerps = frac(samplePoint);
	int3 iSamplePoint = (int3)samplePoint;
	float3 lower = float3(FALLOFF_DATA[iSamplePoint.r].r, FALLOFF_DATA[iSamplePoint.g].g, FALLOFF_DATA[iSamplePoint.b].b);
	float3 upper = float3(FALLOFF_DATA[iSamplePoint.r + 1].r, FALLOFF_DATA[iSamplePoint.g + 1].g, FALLOFF_DATA[iSamplePoint.b + 1].b);
	return lerp(lower, upper, lerps);
	*/
	float ns2 = -s * s;
	float3 result = 0;
	for (int i = 0; i < NUM_GAUSSIANS; i++) {
		if (g_sss_coeff_sigma2[i].a != 0) {
			result += g_sss_coeff_sigma2[i].rgb * exp(ns2 / g_sss_coeff_sigma2[i].a);
		}
	}
	return result / g_sss_color_tone;
}

float distance(float3 vPosWS, float3 vNormalWS, float NdotL, float3 vPosLightWS, matrix matViewProjLight,
			   Texture2D shadowMap, SamplerState samShadow)
{
	float3 vShrinkedPosWS = vPosWS - vNormalWS * lerp(0.01, 0, saturate(-NdotL));
	float4 vPosPSL4 = mul(float4(vShrinkedPosWS, 1.0), matViewProjLight);
	float2 vPosTeSL = float2(0.5, -0.5) * (vPosPSL4.xy / vPosPSL4.w) + 0.5;
	float shadowDepth = shadowMap.Sample(samShadow, vPosTeSL).r;
	return abs(length(vPosLightWS - vShrinkedPosWS) - shadowDepth);
}

float3 backlit_amount(float3 vPosWS, float3 vNormalWS, float3 L, float3 vPosLightWS, matrix matViewProjLight,
					  Texture2D shadowMap, SamplerState samShadow) {
	float NdotL = dot(vNormalWS, L);
	float s = 0.3 * MM_PER_LENGTH * distance(vPosWS, vNormalWS, NdotL, vPosLightWS, matViewProjLight, shadowMap, samShadow) * (1.0f / max(0.01f, g_sss_strength));
	float3 atten = trans_atten(s);
	float NdotL2 = saturate(0.3 - NdotL);
	return NdotL2 * atten;
}

PS_OUTPUT_IRRADIANCE PS_Irradiance(PS_INPUT_IRRADIANCE input) {
	PS_OUTPUT_IRRADIANCE output;
	// textured irradiance color
	float4 sample = g_texture.Sample(g_samTexture, input.texCoord);
	float3 tex_color = input.color.rgb * sample.rgb * 2;
	float3 sq_tex_color = sqrt(tex_color);
	float ao = sample.a;

	input.vNormalWS = normalize(input.vNormalWS);

	// calculated pertubated normal
#if USE_NORMAL_MAP == 0
	float3 vNormalWS = calcNormalWS(g_samBump, g_bump, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#else
	float3 vNormalWS = calcNormalFromNormalMap(g_samNormal, g_normalMap, input.texCoord, input.vNormalWS, input.vTangentWS, input.vBinormalWS);
#endif

	// specular light scaling factor
	float rho_s = 1.f;
	// surface roughness
	float m = g_material.roughness;
	// shading
	// global ambient
	float3 totalambient = g_ambient * g_material.ambient;

	float3 irradiance = 0.0;
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
		float NdotL = max(dot(N, L), 0);
		float atten = light_attenuation(input.vPosWS, l);
		// ambient
		float3 ambient = l.ambient * g_material.ambient;
		// diffuse
		float diffuseLight = NdotL;
		float3 diffuse = atten * l.diffuse * g_material.diffuse * diffuseLight;
		// specular
		float3 H = normalize(L + V);
		float specularLight = rho_s * CookTorrance(N, V, L, H, m);
		// look up shadow map for light amount
		float lightAmount = light_amount(input.vPosWS, g_shadowMaps[i], g_samShadow, ldepth, g_matViewProjLights[i]);
		// fresnel transmittance for diffuse irradiance
		float fresnel_trans = saturate(1 - rho_s * g_attenuation.SampleLevel(g_samAttenuation, float2(NdotL, m), 0).r);

		// calculate transmittance from back of the object
		float3 backlitAmount = backlit_amount(input.vPosWS, input.vNormalWS, L, l.position, g_matViewProjLights[i],
			g_shadowMaps[i], g_samShadowDepth) * (1 - lightAmount) * g_sss_intensity;
		float3 backlit = atten * backlitAmount * l.diffuse * g_material.diffuse;

		// putting them together
		totalambient += ambient;
		specular += atten * l.specular * g_material.specular * specularLight * lightAmount;
		irradiance += diffuse * lightAmount * fresnel_trans + backlit;
	}

	// specular reflection amount from view angle
	float specRef = g_attenuation.SampleLevel(g_samAttenuation, float2(dot(N, V), m), 0).r;

	// consider ambient occlusion, reflection & transmittance
	totalambient *= ao;
	//specular += totalambient * rho_s * specRef;
	irradiance += totalambient * FRESNEL_TRANS_TOTAL;

	// calculate fresnel outgoing factor
	float fresnel_trans_view = saturate(1 - rho_s * specRef);

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
	float kSizeX = getKernelSize(input.depth, drx) * getDDXDepthPenalty(input.depth) * g_sss_strength;
	float kSizeY = getKernelSize(input.depth, dry) * getDDYDepthPenalty(input.depth) * g_sss_strength;

	output.irradiance = float4(sq_tex_color * irradiance, g_sss_intensity);
	output.albedo = float4(sq_tex_color * fresnel_trans_view, 1.0);
	output.diffuseStencil = float4(kSizeX, kSizeY, 0.0, 0.0);
	output.specular = float4(specular, 1.0);

	return output;
}

float4 PS_Irradiance_NoGaussian(PS_INPUT_IRRADIANCE input) : SV_Target {
	PS_OUTPUT_IRRADIANCE output = PS_Irradiance(input);
	// combine immediately
	float3 color = output.irradiance.rgb * output.albedo.rgb * g_sss_color_tone + output.specular.rgb;
	return float4(color, 1.0);
}

float4 PS_Irradiance_NoGaussian_AA(PS_INPUT_IRRADIANCE input) : SV_Target {
	float3 color = PS_Irradiance_NoGaussian(input).rgb;
	return lum_output_linear(color);
}
