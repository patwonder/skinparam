// Information about the near and far clipping plane

static const float DEPTH_NEAR = 0.1;
static const float DEPTH_FAR = 20.0;

// converts from camera space depth to projection space depth
float getDepthPS(float depthCS) {
	return (depthCS - DEPTH_NEAR) / (DEPTH_FAR - DEPTH_NEAR);
}

// converts from projection space depth to camera space depth
float getDepthCS(float depthPS) {
	return lerp(DEPTH_NEAR, DEPTH_FAR, depthPS);
}
