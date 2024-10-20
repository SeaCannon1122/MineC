#version 430 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out int text_id;

void main() {
    text_id = gl_PrimitiveID / 2;

    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
