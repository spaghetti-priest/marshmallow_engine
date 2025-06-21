#ifndef MARSHMALLOW_GL_H

#include "sdl_marshmallow.h"
// #include "pepsimania_math.h"

// @Remove: Remove this m_draw.h include in the future This is not good archetecture.
#include "marshmallow_draw.h"

// @Remove: remove this inlieu in order to implement orthogrpahic matrix
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// #include "SDL_vulkan.h"
#include <glad/glad.h>

// @Todo: Remove this evil header and its malicious functions
#include <vector>
#include <cstring>

typedef struct OpenGL {
   SDL_GLContext gl_context;
   unsigned int screen_shader;
} OpenGL;

typedef struct BufferObject {
   u32 vao;
   u32 vbo;
   u32 ebo;
   u32 instance_vbo;
   u32 instance_count;
} BufferObject;

function bool init_sdl_opengl(SDL_Window *window, u32 screen_w, u32 screen_h);
function void gl_begin_frame();
function void gl_render_frame();
function void gl_swap_framebuffers(SDL_Window *window);
function void gl_cleanup();

function GLuint gl_create_shader_program(const char *vertex_path, const char *frag_path);

function void gl_push_quad_immediate(V2S min, V2S max, V4 color);
function void gl_push_quad_instanced(V2S min, V2S max, V4 color,
                                     V2S *translations, u32 instance_count);

function void gl_initialize_mesh_buffers(Mesh *mesh);
function u32  gl_upload_texture_from_file(const char *path, const std::string directory, bool gamma);

#define MARSHMALLOW_GL_H
#endif
