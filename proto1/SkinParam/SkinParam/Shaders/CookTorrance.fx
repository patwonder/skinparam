// Cook-Torrance specular reflection

static const float PI = 3.141592654;
static const float INDEX = 1.44;
static const float FRESNEL_R0 = (1 - INDEX) * (1 - INDEX) / ((1 + INDEX) * (1 + INDEX));

float fresnel_term(float NdotL) {
	return FRESNEL_R0 + (1 - FRESNEL_R0) * pow(1.0 - NdotL, 5.0);
}

float geometry_term(float NdotL, float NdotH, float NdotV, float VdotH) {
	float base = 2 * NdotH / VdotH;
	float masking = base * NdotV;
	float shadowing = base * NdotL;
	return saturate(min(masking, shadowing));
}

float distribution_term(float NdotH, float rms_slope) {
	// using beckman distribution
	float exponent = tan(acos(NdotH)) / rms_slope;
	exponent *= -exponent;
	float cosb2 = NdotH * NdotH;
	float denominator = 4 * rms_slope * rms_slope * cosb2 * cosb2;
	return exp(exponent) / denominator;
}

// assume normalized params
float3 CookTorrance(float3 N, float3 V, float3 L, float3 H, float rms_slope) {
	float NdotL = dot(N, L);
	float NdotH = dot(N, H);
	float NdotV = dot(N, V);
	float VdotH = dot(V, H);

	float D = distribution_term(NdotH, rms_slope);
	float G = geometry_term(NdotL, NdotH, NdotV, VdotH);
	float F = fresnel_term(NdotL);

	return clamp(D * G * F / (PI * NdotL * NdotV), 0, 2);
}
