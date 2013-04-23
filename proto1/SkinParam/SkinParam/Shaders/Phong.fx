// Implements the phong shading model

#include "Lighting.fx"

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float3 normal : TEXCOORD0;
	float3 worldPos : TEXCOORD1;
};

VS_OUTPUT VS(float4 pos : POSITION, float4 color : COLOR, float3 normal : NORMAL) {
	VS_OUTPUT output;
	float4 wpos4 = mul(pos, g_matWorld);
	output.worldPos = wpos4.xyz / wpos4.w;
	output.pos = mul(wpos4, g_matViewProj);
	output.color = color;
	output.normal = mul(float4(normal, 0.0), g_matWorld).xyz;
	return output;
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

float4 PS(VS_OUTPUT input) : SV_Target {
	float3 color = phong_lighting(g_material, g_ambient, g_lights, g_posEye, input.worldPos, input.color.rgb, input.normal);
	return float4(color, 1.0);
}
