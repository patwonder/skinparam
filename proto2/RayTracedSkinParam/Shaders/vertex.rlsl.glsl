attribute vec3 attr_position;
attribute vec3 attr_normal;
attribute vec3 attr_tangent;
attribute vec3 attr_binormal;
attribute vec2 attr_texCoord;

transformed varying vec3 var_normal;
transformed varying vec3 var_tangent;
transformed varying vec3 var_binormal;
varying vec2 var_texCoord;

void main() {
	rl_Position = vec4(attr_position, 0.0);
	var_normal = attr_normal;
	var_tangent = attr_tangent;
	var_binormal = attr_binormal;
	var_texCoord = attr_texCoord;
}
