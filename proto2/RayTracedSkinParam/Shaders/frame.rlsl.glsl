/*// RLSL
void main() {
	vec3 color = vec3(rl_FrameCoord.x/rl_FrameSize.x,
					  rl_FrameCoord.y/rl_FrameSize.y,
					  1.0 - (rl_FrameCoord.x + rl_FrameCoord.y) / (rl_FrameSize.x + rl_FrameSize.y));
	accumulate(color);
}
*/
rayattribute vec3 color;

void setup() {
	rl_OutputRayCount = 1;
}

// Inverse view-projection matrix
uniform mat4 g_matInvViewProj;

void main() {
	// calculate the normalized projection-space coordinate
	vec4 vOriginPS = vec4(2.0 * (rl_FrameCoord.xy / rl_FrameSize.xy - 0.5), 0.0, 1.0);
	vec4 vDestPS = vec4(vOriginPS.xy, 1.0, 1.0);
	// calculate the world-space coordinate for the ray
	vec4 vOriginWS4 = g_matInvViewProj * vOriginPS;
	vec4 vDestWS4 = g_matInvViewProj * vDestPS;
	// emit the ray
	createRay();
	rl_OutRay.origin = vOriginWS4.xyz / vOriginWS4.w;
	rl_OutRay.direction = vDestWS4.xyz / vDestWS4.w - rl_OutRay.origin;
	rl_OutRay.color = vec3(1.0);
	emitRay();
}
