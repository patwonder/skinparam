// Ray shader for the light
rayattribute vec3 color;

void setup() {

}

uniform vec3 g_color;

void main() {
	accumulate(g_color * rl_InRay.color);
}
