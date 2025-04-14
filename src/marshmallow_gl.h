#ifndef MARSHMALLOW_GL_H

#include "sdl_marshmallow.h"
// #include "pepsimania_math.h"

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

function bool init_opengl(SDL_Window *window, u32 screen_w, u32 screen_h);
function void gl_render_frame();
function void gl_swap_framebuffers(SDL_Window *window);
function void gl_cleanup();

#define MARSHMALLOW_GL_H
#endif
