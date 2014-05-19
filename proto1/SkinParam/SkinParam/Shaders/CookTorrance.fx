
/*
    Copyright(c) 2013-2014 Yifan Wu.

    This file is part of SkinParam.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

// Cook-Torrance specular reflection

static const float PI = 3.141592654;
static const float INDEX = 1.4;
static const float FRESNEL_R0 = (1 - INDEX) * (1 - INDEX) / ((1 + INDEX) * (1 + INDEX));

float fresnel_term(float cosA) {
	//return FRESNEL_R0 + (1 - FRESNEL_R0) * pow(1.0 - cosA, 5.0);
	float sint2 = (1 - cosA * cosA) / (INDEX * INDEX);
	float cost = sqrt(1 - sint2);
	float parl = (INDEX * cosA - cost) / (INDEX * cosA + cost);
	float perp = (cosA - INDEX * cost) / (cosA + INDEX * cost);
	return 0.5 * (parl * parl + perp * perp);
}

float geometry_term(float NdotL, float NdotH, float NdotV, float VdotH) {
	float base = 2 * NdotH / VdotH;
	float masking = base * NdotV;
	float shadowing = base * NdotL;
	return saturate(min(masking, shadowing));
}

float distribution_term(float NdotH, float rms_slope) {
	// using beckmann distribution
	float rcpRMS2 = 1 / (rms_slope * rms_slope);
	float c2 = NdotH * NdotH;
	float d = PI * c2 * c2;
	if (d == 0) return 0;
	float e = (c2 - 1) * rcpRMS2 / c2;
	return rcpRMS2 * exp(e) / d;
}

// assume normalized params
float CookTorrance(float3 N, float3 V, float3 L, float3 H, float rms_slope) {
	float NdotL = dot(N, L);
	float NdotH = dot(N, H);
	float NdotV = dot(N, V);
	float VdotH = dot(V, H);

	float D = distribution_term(NdotH, rms_slope);
	float G = geometry_term(NdotL, NdotH, NdotV, VdotH);
	// Should use VdotH for the fresnel term for rough surfaces,
	// as suggested in http://http.developer.nvidia.com/GPUGems3/gpugems3_ch14.html
	float F = saturate(fresnel_term(VdotH));
	// Cook-Torrance
	return D * G * F / (4 * NdotV);

	// Kelemen/Szirmay-Kalos Specular
	//return max(D * F * NdotL / dot(L + V, L + V), 0);
}
