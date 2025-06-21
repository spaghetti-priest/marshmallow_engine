#version 450 core

layout(location = 0) in vec3 in_positions;
layout(location = 1) in vec3 in_normals;
layout(location = 2) in vec2 in_texcoords;
layout(location = 3) in vec3 in_tangents;
layout(location = 4) in vec3 in_bitangent;

// out vec4 frag_color;
out vec2 out_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
   mat4 MVP       = proj * view * model;
   gl_Position    = MVP * vec4(in_positions, 1.0);
   out_texcoords  = in_texcoords;
   // frag_color     = in_colors;
}
