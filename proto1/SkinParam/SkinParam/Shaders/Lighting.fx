/**
 * General buffer for 3D lighting and transform
 */

#include "CookTorrance.fx"
#include "NearFar.fx"

static const uint NUM_LIGHTS = 2;
static const uint SM_SIZE = 2048;
static const float SHADOW_VARIANCE_MIN = 1e-4;
static const float SHADOW_LIGHTAMOUNT_MIN = 0.5;
static const float RMS_SLOPE = 0.25;

Texture2D g_texture : register(t0);
Texture2D g_bump : register(t1);
Texture2D g_normalMap : register(t2);
Texture2D g_shadowMaps[NUM_LIGHTS] : register(t3);
SamplerState g_samTexture : register(s0);
SamplerState g_samBump : register(s1);
SamplerState g_samNormal : register(s2);
SamplerState g_samShadow : register(s3);

struct Light {
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float3 attenuation;
	float3 position;
};

cbuffer Transform : register(b0) {
	matrix g_matWorld;
	matrix g_matView;
	matrix g_matViewProj;
	matrix g_matViewLights[NUM_LIGHTS];
	matrix g_matViewProjLights[NUM_LIGHTS];
	matrix g_matViewProjCamera;
	float3 g_posEye;
	float g_fAspectRatio;
	float4 g_vFrustumPlaneEquation[4]; // View frustum plane equations : x*x0+y*y0+z*z0+w0=0
};

cbuffer Lighting : register(b1) {
	Light g_lights[NUM_LIGHTS];
	float3 g_ambient;
};

struct Material {
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float shininess;
	float3 emmisive;
	float bump_multiplier;
};

cbuffer Material : register(b2) {
	Material g_material;
};

float light_attenuation(float3 pos, Light l) {
	float dist = distance(l.position, pos);
	return 1 / (l.attenuation.x + l.attenuation.y * dist + l.attenuation.z * dist * dist);
}

float dampedLightAmount(float lightAmount) {
	return max(0.0, lightAmount - SHADOW_LIGHTAMOUNT_MIN) / (1.0 - SHADOW_LIGHTAMOUNT_MIN);
}

float light_amount(float3 worldPos, Texture2D shadowMap, SamplerState samShadow, matrix matViewLight, matrix matViewProjLight) {
	float4 vPosPSL4 = mul(float4(worldPos, 1.0), matViewProjLight);

	// Texture space point location
	float2 vPosTeSL = 0.5 * (vPosPSL4.xy / vPosPSL4.w) + 0.5;
	vPosTeSL.y = 1.0 - vPosTeSL.y;

	float lightAmount;
	if (vPosTeSL.x < 0.0 || vPosTeSL.x > 1.0 || vPosTeSL.y < 0.0 || vPosTeSL.y > 1.0) {
		// out of shadow map, lit the texel
		lightAmount = 1.0;
	} else {
		float2 vsmSample = shadowMap.Sample(samShadow, vPosTeSL).rg;
		float depth = vPosPSL4.z / vPosPSL4.w;
		float meanDepth = vsmSample.r;
		float meanDepthSquared = vsmSample.g;
		if (depth <= meanDepth) {
			lightAmount = 1.0;
		} else {
			// Calculate variance, addressing numerical instability
			float variance = max(SHADOW_VARIANCE_MIN, meanDepthSquared - meanDepth * meanDepth);
			float depthDiff = depth - meanDepth;

			lightAmount = dampedLightAmount(variance / (variance + depthDiff * depthDiff));
		}
	}
	return lightAmount;
}

float3 phong_shadow(Material mt, float3 ambient, Light lights[NUM_LIGHTS],
					float3 eyePos, float3 worldPos, float3 color, float3 normal,
					Texture2D shadowMaps[NUM_LIGHTS], SamplerState samShadow, matrix matViewLights[NUM_LIGHTS], matrix matViewProjLights[NUM_LIGHTS])
{
	// global ambient
	float3 result = color * ambient * mt.ambient;
	// emmisive
	result += mt.emmisive;

	float3 N = normalize(normal);
	float3 V = normalize(eyePos - worldPos);
	// lights
	for (uint i = 0; i < NUM_LIGHTS; i++) {
		Light l = lights[i];

		// light vector & attenuation
		float3 L = normalize(l.position - worldPos);
		float NdotL = dot(N, L);
		float atten = light_attenuation(worldPos, l);
		// ambient
		float3 ambient = l.ambient * mt.ambient;
		// diffuse
		float diffuseLight = max(NdotL, 0);
		float3 diffuse = atten * l.diffuse * mt.diffuse * diffuseLight;
		// specular
		float3 H = normalize(L + V);
		float specularLight = saturate(CookTorrance(N, V, L, H, RMS_SLOPE));//saturate(pow(max(dot(N, H), 0), mt.shininess));
		float3 specular = atten * l.specular * mt.specular * specularLight;
		// look up shadow map for light amount
		float lightAmount = light_amount(worldPos, shadowMaps[i], samShadow, matViewLights[i], matViewProjLights[i]);

		// putting them together
		result += color * (ambient + diffuse * lightAmount) + specular * lightAmount;
	}

	return result;
}

float3 phong_lighting(Material mt, float3 ambient, Light lights[NUM_LIGHTS],
					  float3 eyePos, float3 worldPos, float3 color, float3 normal)
{
	// global ambient
	float3 result = color * ambient * mt.ambient;
	// emmisive
	result += mt.emmisive;

	float3 N = normalize(normal);
	float3 V = normalize(eyePos - worldPos);
	// lights
	for (uint i = 0; i < NUM_LIGHTS; i++) {
		Light l = lights[i];
		// light vector & attenuation
		float3 L = normalize(l.position - worldPos);
		float NdotL = dot(N, L);
		float atten = light_attenuation(worldPos, l);
		// ambient
		float3 ambient = l.ambient * mt.ambient;
		// diffuse
		float diffuseLight = max(NdotL, 0);
		float3 diffuse = atten * l.diffuse * mt.diffuse * diffuseLight;
		// specular
		float3 H = normalize(L + V);
		float specularLight = saturate(pow(max(dot(N, H), 0), mt.shininess));
		float3 specular = atten * l.specular * mt.specular * specularLight;

		// putting them together
		result += color * (ambient + diffuse) + specular;
	}

	return result;
}

float2 getScreenSpacePosition(float3 vPosWS, matrix matViewProj) {
	float4 vPosPS4 = mul(float4(vPosWS, 1.0), matViewProj);
	float2 vPosPS = vPosPS4.xy / vPosPS4.w;
	vPosPS.x *= g_fAspectRatio;
	return vPosPS;
}
