#version 450 core

// in vec4 frag_color;
in vec2 tex_coord;
out vec4 out_color;

uniform sampler2D text_sampler;
uniform vec3 text_color;

void main() {
   vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text_sampler, tex_coord).r);
   out_color = vec4(text_color, 1.0) * sampled;
}