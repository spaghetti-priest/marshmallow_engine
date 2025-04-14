#version 450 core

layout(location = 0) in vec2 in_positions;
layout(location = 1) in vec3 in_colors;
layout(location = 2) in vec2 in_offsets;

out vec4 frag_color;

uniform mat4 orthographic;
uniform mat4 model;
void main() {
   gl_Position = orthographic * vec4(in_positions, 0.0, 1.0);
   frag_color = vec4(in_colors, 1.0);
}