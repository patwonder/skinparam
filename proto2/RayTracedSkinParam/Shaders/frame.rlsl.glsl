/*// RLSL
void main() {
	vec3 color = vec3(rl_FrameCoord.x/rl_FrameSize.x,
					  rl_FrameCoord.y/rl_FrameSize.y,
					  1.0 - (rl_FrameCoord.x + rl_FrameCoord.y) / (rl_FrameSize.x + rl_FrameSize.y));
	accumulate(color);
}
*/
rayattribute vec3 color;

#define AA_LEVEL 1

void setup() {
	rl_OutputRayCount = AA_LEVEL * AA_LEVEL;
}

// Inverse view-projection matrix
uniform mat4 g_matInvViewProj;

void main() {
	vec2 rcpScreenSize = vec2(1.0) / rl_FrameSize.xy;
	vec2 halfRcpScreenSize = rcpScreenSize * 0.5;
	vec2 stepSize = halfRcpScreenSize / float(AA_LEVEL);
	float weight = 1.0 / float(AA_LEVEL * AA_LEVEL);
	for (int i = 0; i < AA_LEVEL; i++) {
		for (int j = 0; j < AA_LEVEL; j++) {
			// calculate the normalized projection-space coordinate
			vec2 vFrameCoordPS = rl_FrameCoord.xy - vec2(0.5) + vec2(float(i) + 0.5, float(j) + 0.5) / float(AA_LEVEL);
			vec4 vOriginPS = vec4(2.0 * (vFrameCoordPS / rl_FrameSize.xy - 0.5), 0.0, 1.0);
			vec4 vDestPS = vec4(vOriginPS.xy, 1.0, 1.0);
			// calculate the world-space coordinate for the ray
			vec4 vOriginWS4 = g_matInvViewProj * vOriginPS;
			vec4 vDestWS4 = g_matInvViewProj * vDestPS;
			// emit the ray
			createRay();
			rl_OutRay.origin = vOriginWS4.xyz / vOriginWS4.w;
			rl_OutRay.direction = vDestWS4.xyz / vDestWS4.w - rl_OutRay.origin;
			rl_OutRay.color = vec3(weight);
			emitRay();
		}
	}
}
