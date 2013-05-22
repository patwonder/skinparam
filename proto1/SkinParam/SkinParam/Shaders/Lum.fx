// Luminance calculation

float4 lum_output_linear(float3 color) {
	return float4(color, sqrt(dot(color, float3(0.299, 0.587, 0.114))));
}
