// Bump -> normal calculation

static const float BUMP_SIZE = 2048;
static const float BUMP_PIXEL_DIST = 0.05;

float3 calcNormalWS(SamplerState samBump, Texture2D bump, float2 texCoord, float3 vNormalWS, float3 vTangentWS, float3 vBinormalWS) {
	// normalize stuff
	vNormalWS = normalize(vNormalWS);
	vTangentWS = normalize(vTangentWS);
	vBinormalWS = normalize(vBinormalWS);

	// matrix to transform vectors from tangent space to world space
	// (assume orthorgonalized normal, tangent and binormal vectors)
	float3x3 matTanToWorld = float3x3(vTangentWS, vBinormalWS, vNormalWS);

	// calculate tangent space normal by averaging nearby triangle normals
	float bump11 = bump.Sample(samBump, texCoord).r;
	float3 bump01 = float3(-BUMP_PIXEL_DIST, 0, bump.Sample(samBump, texCoord + float2(-1.0 / BUMP_SIZE, 0)).r - bump11);
	float3 bump21 = float3(BUMP_PIXEL_DIST, 0, bump.Sample(samBump, texCoord + float2(1.0 / BUMP_SIZE, 0)).r - bump11);
	float3 bump10 = float3(0, -BUMP_PIXEL_DIST, bump.Sample(samBump, texCoord + float2(0, -1.0 / BUMP_SIZE)).r - bump11);
	float3 bump12 = float3(0, BUMP_PIXEL_DIST, bump.Sample(samBump, texCoord + float2(0, 1.0 / BUMP_SIZE)).r - bump11);
	float3 vNormalTS = cross(bump01, bump10) + cross(bump10, bump21) + cross(bump21, bump12) + cross(bump12, bump01);

	// transform to world space
	float3 vFinalNormalWS = mul(vNormalTS, matTanToWorld);

	return vFinalNormalWS;
}

float3 calcNormalFromNormalMap(SamplerState samNormal, Texture2D normal, float2 texCoord, float3 vNormalWS, float3 vTangentWS, float3 vBinormalWS) {
	// normalize stuff
	vNormalWS = normalize(vNormalWS);
	vTangentWS = normalize(vTangentWS);
	vBinormalWS = normalize(vBinormalWS);

	// matrix to transform vectors from tangent space to world space
	// (assume orthorgonalized normal, tangent and binormal vectors)
	float3x3 matTanToWorld = float3x3(vTangentWS, vBinormalWS, vNormalWS);

	// calculate tangent space normal by sampling the normal map
	float3 vNormalTS = float3(normal.Sample(samNormal, texCoord).rg, 1.0);

	// transform to world space
	float3 vFinalNormalWS = mul(vNormalTS, matTanToWorld);

	return vFinalNormalWS;
}
