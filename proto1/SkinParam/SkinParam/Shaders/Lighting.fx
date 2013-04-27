/**
 * General buffer for 3D lighting and transform
 */

Texture2D g_texture : register(t0);
SamplerState g_samTexture : register(s0);

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
