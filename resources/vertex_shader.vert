#version 450

vec2 vertices[3] = {
	vec2(0.57735, 0.33333),
	vec2(0, -0.66666),
	vec2(-0.57735, 0.33333)
};

void main() {

	gl_Position = vec4(vertices[gl_VertexIndex].xy, 1.0, 1.0);

}