#version 450 core

out vec4 frag_color;

in vec2 out_texcoords;

uniform sampler2D texture_normal;
uniform sampler2D texture_diffuse;
//uniform sampler2D texture_diffuse1;
//uniform sampler2D texture_specular1;

void main () {
   // vec3 diffuse_texture = texture(texture_diffuse, out_texcoords).rgb;
   // vec3 normal_texture  = texture(texture_normal, out_texcoords).rgb;
   frag_color = vec4(texture(texture_diffuse, out_texcoords).rgb, 1.0f);
}