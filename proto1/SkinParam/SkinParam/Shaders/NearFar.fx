// Information about the near and far clipping plane

static const float DEPTH_NEAR = 0.1;
static const float DEPTH_FAR = 30.0;

float normalizeDepth(float depth) {
	return (depth - DEPTH_NEAR) / (DEPTH_FAR - DEPTH_NEAR);
}
