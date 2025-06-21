#version 450 core

layout(location = 0) in vec3 in_positions;
layout(location = 1) in vec4 in_colors;
layout(location = 2) in vec2 in_offsets;

out vec4 frag_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
   gl_Position = proj * view * model * vec4(in_positions.x + in_offsets.x,
                       in_positions.y + in_offsets.y,
                       in_positions.z,
                       1.0);
   // gl_Position = proj * view * model * vec4(in_positions, 1.0);

   frag_color = in_colors;
}
