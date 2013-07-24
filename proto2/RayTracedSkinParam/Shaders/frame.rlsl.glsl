// RLSL
void main() {
	vec3 color = vec3(rl_FrameCoord.x/rl_FrameSize.x,
					  rl_FrameCoord.y/rl_FrameSize.y,
					  0.0);
	accumulate(color);
}
