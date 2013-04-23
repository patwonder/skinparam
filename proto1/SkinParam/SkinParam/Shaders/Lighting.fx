/**
 * General buffer for 3D lighting and transform
 */

struct Light {
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float3 attenuation;
	float3 position;
};

cbuffer Transform : register(b0) {
	matrix g_matWorld;
	matrix g_matViewProj;
	float3 g_posEye;
};

static const uint NUM_LIGHTS = 2;

cbuffer Lighting : register(b1) {
	Light g_lights[NUM_LIGHTS];
	float3 g_ambient;
};

struct Material {
	float3 ambient;
	float3 diffuse;
	float3 specular;
	float3 emmisive;
	float shininess;
};

cbuffer Material : register(b2) {
	Material g_material;
};

float light_attenuation(float3 pos, Light l) {
	float dist = distance(l.position, pos);
	return 1 / (l.attenuation.x + l.attenuation.y * dist + l.attenuation.z * dist * dist);
}
