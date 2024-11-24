#version 450

vec4 vertices[4] = {
	vec4(-0.5, -0.5, 0.0, 0.0),
	vec4(-0.5, 0.5, 0.0, 1.0),
	vec4(0.5, -0.5, 1.0, 0.0),
	vec4(0.5, 0.5, 1.0, 1.0),


};

int indicies[6] = {
	1, 2, 0, 1, 3, 2
};

layout (location = 0) out vec2 uv;

void main() {

	gl_Position = vec4(vertices[indicies[gl_VertexIndex]].xy, 1.0, 1.0);

	uv = vertices[indicies[gl_VertexIndex]].zw;

}