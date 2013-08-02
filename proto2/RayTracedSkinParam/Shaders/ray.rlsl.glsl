void setup() {

}

varying vec3 var_normal;
varying vec2 var_texCoord;

uniform sampler2D g_texture;

vec4 srgb(sampler2D sampler, vec2 texCoord) {
	vec4 color = texture2D(sampler, texCoord);
	return vec4(pow(color.rgb, vec3(2.2)), color.a);
}

void main() {
	accumulate(srgb(g_texture, var_texCoord).rgb);
}
