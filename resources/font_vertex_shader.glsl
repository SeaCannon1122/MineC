#version 430 core

layout(location = 0) in vec2 vertex_pos;
layout(location = 2) in vec4 _transform;
layout(location = 1) in uvec4 _pixel_char;

uniform int screen_width;
uniform int screen_height;

flat out uvec4 pixel_char;

out vec2 pixel_position;

void main() {

    pixel_char = _pixel_char;

    pixel_position = vec2(vertex_pos.x * float((int(_transform.z) + 1) / 2), vertex_pos.y * 8);


    gl_Position = vec4(
       (vertex_pos.x * float( (int(_transform.z)+ 1) / 2 ) * _transform.w + _transform.x) * 2 / float(screen_width ) - 1.0,
       (vertex_pos.y * 8                                   * _transform.w + _transform.y) * 2 / float(screen_height) - 1.0, 
       1.0, 
       1.0
   );
}