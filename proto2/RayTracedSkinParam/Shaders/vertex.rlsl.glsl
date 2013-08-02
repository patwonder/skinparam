attribute vec3 attr_position;
attribute vec3 attr_normal;
attribute vec2 attr_texCoord;

transformed varying vec3 var_normal;
varying vec2 var_texCoord;

void main() {
	rl_Position = vec4(attr_position, 0.0);
	var_normal = attr_normal;
	var_texCoord = attr_texCoord;
}
