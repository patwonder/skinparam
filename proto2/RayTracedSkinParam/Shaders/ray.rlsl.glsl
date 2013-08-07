// 0 - frame ray
// 1 - occlusion-test ray
// 2 - global illumination ray
rayattribute vec3 color;

#define PI 3.141592654
#define OCCLUSION_RAYS 16

void setup() {
    rl_OutputRayCount[0] = OCCLUSION_RAYS;
    rl_OutputRayCount[2] = 2;
}

varying vec3 var_normal;
varying vec3 var_tangent;
varying vec3 var_binormal;
varying vec2 var_texCoord;

uniform sampler2D g_texture;
uniform sampler2D g_normal;

// defining the spherical light
uniformblock g_light {
    vec4 position_radius;
    primitive prim;
};

uniform primitive g_environmentPrimitive;
vec4 srgb(sampler2D sampler, vec2 texCoord) {
    vec4 color = texture2D(sampler, texCoord);
    return vec4(pow(color.rgb, vec3(2.2)), color.a);
}

vec3 calcNormalFromNormalMap(sampler2D sampler, vec2 texCoord, vec3 vNormalWS, vec3 vTangentWS, vec3 vBinormalWS) {
    // normalize stuff
	vNormalWS = normalize(vNormalWS);
    vTangentWS = normalize(vTangentWS);
    vBinormalWS = normalize(vBinormalWS);
    // matrix to transform vectors from tangent space to world space
	// (assume orthorgonalized normal, tangent and binormal vectors)
	mat3 matTanToWorld = mat3(vTangentWS, vBinormalWS, vNormalWS);
    // calculate tangent space normal by sampling the normal map
	vec3 vNormalTS = vec3(texture2D(sampler, texCoord).rg, 1.0);
    // transform to world space
	vec3 vFinalNormalWS = matTanToWorld * vNormalTS;
    return vFinalNormalWS;
}

int gray(int i) {
	return i ^ (i >> 1);
}

void main() {
    vec3 tex_color = srgb(g_texture, var_texCoord).rgb;
    vec3 vNormalWS = normalize(calcNormalFromNormalMap(g_normal, var_texCoord, var_normal, var_tangent, var_binormal));
    // occlusion-test rays
    vec3 lightPos = g_light.position_radius.xyz;
    float lightRadius = g_light.position_radius.w;
	vec3 mainDirection = lightPos - rl_IntersectionPoint;
    vec3 lightXAxis = normalize(cross(vec3(1.0, 0.0, 0.0), mainDirection));
    vec3 lightYAxis = normalize(cross(lightXAxis, mainDirection));
    vec4 targets[OCCLUSION_RAYS];
	float totalWeight = 0.0;
	for (int i = 0; i < OCCLUSION_RAYS; i++) {
		float angle = 2.0 * PI / float(OCCLUSION_RAYS) * float(i);
		vec3 targetPos = lightPos + float(gray(gray(gray(gray(gray(i)))))) / float(OCCLUSION_RAYS) * lightRadius * (lightXAxis * cos(angle) + lightYAxis * sin(angle));
		float targetWeight = 1.0;
		totalWeight += targetWeight;
		targets[i] = vec4(targetPos, targetWeight);
	};
    for (int i = 0; i < OCCLUSION_RAYS; i++) {
        vec3 target = targets[i].xyz;
		float weight = targets[i].w / totalWeight;
		float NdotL = clamp(dot(vNormalWS, normalize(target - rl_IntersectionPoint)), 0.0, 1.0);
        createRay();
        rl_OutRay.origin = rl_IntersectionPoint;
        rl_OutRay.direction = target - rl_IntersectionPoint;
        rl_OutRay.color *= weight * tex_color * NdotL;
        rl_OutRay.rayClass = 1;
        rl_OutRay.occlusionTest = true;
        rl_OutRay.maxT = length(target - rl_IntersectionPoint);
        rl_OutRay.defaultPrimitive = g_light.prim;
        emitRay();
    }

	// TODO: global illumination rays
}
