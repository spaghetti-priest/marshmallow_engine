#version 450 core

layout(location = 0) in vec3 in_positions;
// layout(location = 1) in vec4 in_colors;
// layout(location = 2) in vec2 in_offsets;
layout(location = 1) in vec2 in_tex_coord;

// out vec4 frag_color;
out vec2 tex_coord;

uniform mat4 orthographic;
uniform mat4 model;

void main() {
   gl_Position = orthographic * vec4(in_positions, 1.0);
   tex_coord = in_tex_coord;
   // frag_color = in_colors;
}