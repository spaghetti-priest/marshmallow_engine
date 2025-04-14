#include <string>

float vertices[] = {
   -0.5f, -0.5f, 0.0f, // left
   0.5f, -0.5f, 0.0f, // right
   0.0f,  0.5f, 0.0f  // top
};

float quad_vertices[] = {
     0.25f,  0.25f, 0.0f,  1.0f, 1.0f, 1.0f, // top right
     0.25f, -0.25f, 0.0f,  1.0f, 0.0f, 1.0f,// bottom right
    -0.25f, -0.25f, 0.0f,  1.0f, 1.0f, 1.0f,// bottom left
    -0.25f,  0.25f, 0.0f,  1.0f, 0.0f, 1.0f// top left
};

const u32 index_count = 6;
u32 indices[index_count] = {
   0, 1, 3, 1, 2, 3
};

typedef struct Shader {
   u32 vertex_shader;
   u32 frag_shader;
   u32 compute_shader;
   u32 program_id;
} Shader;

typedef struct Vertex {
   V3 pos;
   V3 color;
} Vertex;

u32 triangle_shader;
u32 debug_screen_space_shader;
SDL_GLContext gl_context;

// u32 triangle_vao = 0;
// u32 triangle_vbo = 0;
// u32 triangle_ebo = 0;
// u32 quad_instance_vbo = 0;

// @Temporary: debug_read_file does not check for null termination
function MarshFile
gl_read_shader_file (const char *filename)
{
   MarshFile gl_res = {};
   FILE *f = fopen(filename, "rb");
   if(!f)
   f = fopen(filename, "rb");
   const char *strip_name = strrchr(filename, '/');
   if (!f) {
      printf("[ERROR]: Failed to open file: [%s]\n", strip_name);
      return gl_res;
   }

   fseek(f, 0, SEEK_END);
   u32 filesize = ftell(f);
   fseek(f, 0, SEEK_SET);

   char *filecontents  = (char*)malloc(sizeof(char*) * filesize + 1);
   u32 bytes_read      = fread((void*)filecontents, sizeof(char*), filesize, f);

   // if (bytes_read != (filesize / 8)) {
   //    printf("[ERROR]: Bytes read does not equal total filesize, possibly corrupted file!\n");
   //    return gl_res;
   // }
   filecontents[filesize] = '\0';
   gl_res.contents    = (void*)filecontents;
   gl_res.size        = filesize;
   fclose(f);

   return gl_res;
}

function GLuint
compile_shader (const char *filename, GLenum type)
{
   char *buf;
   int length, success;
   char error_buf[1024];
   GLenum error_enum;

   MarshFile shader_file   = gl_read_shader_file(filename);
   GLuint shader           = glCreateShader(type);

   glShaderSource(shader, 1, (const GLchar**)&shader_file.contents, NULL);
   glCompileShader(shader);
   glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

   // if ( !success ) {
   //   glGetShaderInfoLog(shader, 1024, NULL, error_buf);
   //   fprintf(stderr, "SHADER ERROR: Failed to compile shader: [%s]\n", error_buf);
   // }
   return shader;
}

function GLuint
gl_create_shader_program (const char *vertex_path, const char *frag_path)
{
   int success;
   char error_buf[1024];

   u32 vertex_shader    = compile_shader(vertex_path, GL_VERTEX_SHADER);
   u32 frag_shader      = compile_shader(frag_path, GL_FRAGMENT_SHADER);
   u32 shader_program   = glCreateProgram();

   glAttachShader(shader_program, vertex_shader);
   glAttachShader(shader_program, frag_shader);
   glBindFragDataLocation(shader_program, 0, "frag_color");
   glLinkProgram(shader_program);
   glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

   if (!success) {
     glGetProgramInfoLog(shader_program, 1024, NULL, error_buf);
     fprintf(stderr, "[OpenGL]: [ERROR]: Failed to compile shader program: \n%s \n", error_buf);
   } else {
     /*
     vertex_path = strip_file_path(vertex_path);
     frag_path   = strip_file_path(frag_path);
     printf("Shader: %s(%u) loaded successfully\n", vertex_path);
     printf("Shader: %s(%u) loaded successfully\n", frag_path);
     */
   }

   glDeleteShader(vertex_shader);
   glDeleteShader(frag_shader);

   return shader_program;
}

void gl_set_uniform_v2 (u32 ID, const char *name, const glm::vec2 &value)
{
   glUniform2fv(glGetUniformLocation(ID, name), 1, &value[0]);
}

void gl_set_uniform_mat4 (GLuint program, const char *name, glm::mat4 value)
{
   // @Todo @Incomplete: This does not work. Fix!!
   glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &value[0][0]);
   GLenum err = glGetError();
}


// u32 triangle_vao = 0;
// u32 triangle_vbo = 0;
// u32 triangle_ebo = 0;
// u32 quad_vao = 0;
// u32 quad_vbo = 0;
// u32 quad_ebo = 0;
u32 quad_instance_vbo = 0;

typedef struct BufferObject {
   u32 vao;
   u32 vbo;
   u32 ebo;
   u32 instance_vbo;
} BufferObject;

BufferObject quad;
BufferObject debug_quad;

function void
gl_push_quad(V2 min, V2 max, V4 color)
{
   const u32 vertex_count = 4;
   Vertex vertices[vertex_count];
   vertices[0] = {min.x + max.x, min.y + max.y, 0.0f,  color.x, color.y, color.z}; // top right
   vertices[1] = {min.x + max.x, min.y,         0.0f,  color.x, color.y, color.z}; // bottom right
   vertices[2] = {min.x,         min.y,         0.0f,  color.x, color.y, color.z}; // bottom left
   vertices[3] = {min.x,         min.y + max.y, 0.0f,  color.x, color.y, color.z};  // top left

   glGenVertexArrays(1, &quad.vao);
   glBindVertexArray(quad.vao);

   glGenBuffers(1, &quad.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &quad.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 3));
}

function BufferObject
push_quad_immediate (Vertex *vertices, u32 vertex_count)
{
   BufferObject object{};

   glGenVertexArrays(1, &object.vao);
   glBindVertexArray(object.vao);

   glGenBuffers(1, &object.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &object.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 3));

   return object;
}

function BufferObject
push_quad_instanced (Vertex *vertices, u32 vertex_count, V2 *translations, u32 instance_count)
{
   BufferObject object{};

   glGenBuffers(1, &object.instance_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, object.instance_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(V2) * instance_count, &translations[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenVertexArrays(1, &object.vao);
   glBindVertexArray(object.vao);

   glGenBuffers(1, &object.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &object.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 3));

   glBindBuffer(GL_ARRAY_BUFFER, object.instance_vbo);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
   return object;
}

function void
draw_quad (u32 vertex_array_object, u32 instance_count)
{
   // glBindVertexArray(vertex_array_object);
   // glDrawElementsInstanced(GL_TRIANGLES, 0, 4, indices, instance_count);
   // glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instance_count);
}

const u32 instance_count = 18;

function bool
init_opengl (SDL_Window *window, u32 screen_w, u32 screen_h)
{
   SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

   gl_context = SDL_GL_CreateContext(window);

   if (!gladLoadGLLoader((SDL_GL_GetProcAddress))) {
      printf("[ERROR]: Failed to initialize OpenGL extensions loader");
      return 0;
   }

   printf("Vendor:   %s\n", glGetString(GL_VENDOR));
   printf("Renderer: %s\n", glGetString(GL_RENDERER));
   printf("Version:  %s\n", glGetString(GL_VERSION));

   if (SDL_GL_SetSwapInterval(0) != 0) {
      printf("[ERROR]: Failed to get GL_SwapInterval {%s}\n", SDL_GetError());
      return 0;
   };

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MULTISAMPLE);

   // @Todo: When the backbuffer is resized the bb should retrieve
   // the new screen w and h
   glViewport(0, 0, screen_w, screen_h);
   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   triangle_shader = gl_create_shader_program("C:/marshmallow/shaders/shader.vert", "C:/marshmallow/shaders/shader.frag");
   debug_screen_space_shader = gl_create_shader_program("C:/marshmallow/shaders/debug_shader.vert", "C:/marshmallow/shaders/debug_shader.frag");

   const u32 debug_vertex_count = 4;
   Vertex debug_vertices[debug_vertex_count];
   debug_vertices[0] = {{1280.0f,  20.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}; // top right
   debug_vertices[1] = {{1280.0f,  0.0f,  0.0f}, {1.0f, 0.0f, 0.0f}}; // bottom right
   debug_vertices[2] = {{0.0f,     0.0f,  0.0f}, {1.0f, 0.0f, 0.0f}}; // bottom left
   debug_vertices[3] = {{0.0f,     20.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};  // top left

   float x = -1.5f;
   float y = -1.5f;
   float w = 1.0f;
   float h = 1.0f;

   const u32 vertex_count = 4;
   Vertex vertices[vertex_count];
   vertices[0] = {x + w,   y + h, 0.0f,  1.0f, 0.0f, 1.0f}; // top right
   vertices[1] = {x + w,   y,     0.0f,  1.0f, 0.0f, 1.0f}; // bottom right
   vertices[2] = {x,       y,     0.0f,  1.0f, 1.0f, 1.0f}; // bottom left
   vertices[3] = {x,       y + h, 0.0f,  1.0f, 0.0f, 1.0f};  // top left

   V2 translations[instance_count];
   int index = 0;
   // float offset = 0.2f;
   float x_offset = 1;
   float y_offset = 1;

   // @Note: This could be a translation matrix instead
   translations[0].x = x_offset - (w*4);
   translations[1].x = x_offset - (w*3);
   translations[2].x = x_offset - (w*2);
   translations[3].x = x_offset - w;
   translations[4].x = x_offset;
   translations[5].x = x_offset + w;
   translations[6].x = x_offset + (w*2);
   translations[7].x = x_offset + (w*3);
   translations[8].x = x_offset + (w*4);

   translations[9].x = x_offset - (w*4);
   translations[10].x = x_offset - (w*3);
   translations[11].x = x_offset - (w*2);
   translations[12].x = x_offset - w;
   translations[13].x = x_offset;
   translations[14].x = x_offset + w;
   translations[15].x = x_offset + (w*2);
   translations[16].x = x_offset + (w*3);
   translations[17].x = x_offset + (w*4);

   translations[0].y = y_offset;
   translations[1].y = y_offset;
   translations[2].y = y_offset;
   translations[3].y = y_offset;
   translations[4].y = y_offset;
   translations[5].y = y_offset;
   translations[6].y = y_offset;
   translations[7].y = y_offset;
   translations[8].y = y_offset;

   translations[9].y = y_offset +  (h);
   translations[10].y = y_offset + (h);
   translations[11].y = y_offset + (h);
   translations[12].y = y_offset + (h);
   translations[13].y = y_offset + (h);
   translations[14].y = y_offset + (h);
   translations[15].y = y_offset + (h);
   translations[16].y = y_offset + (h);
   translations[17].y = y_offset + (h);

   glGenBuffers(1, &quad.instance_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, quad.instance_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(V2S) * instance_count, &translations[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenVertexArrays(1, &quad.vao);
   glBindVertexArray(quad.vao);

   glGenBuffers(1, &quad.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &quad.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 3));

   glBindBuffer(GL_ARRAY_BUFFER, quad.instance_vbo);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

   // quad = push_quad_instanced(vertices, vertex_count, translations, instance_count);
   // debug_quad = push_quad_immediate(debug_vertices, debug_vertex_count);

   return true;
}

 // debug_draw_rect(buffer, tile->min, tile->max, tile->color.x, tile->color.y, tile->color.z, tile->color.w);

function void
gl_render_frame()
{
   GLenum err;
   // @Hardcoded : width and height are hardcoded
   u32 screen_w = 1280;
   u32 screen_h = 720;

   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(debug_screen_space_shader);
   glm::mat4 ortho_projection = glm::mat4(1.0);
   glm::mat4 ortho_model      = glm::mat4(1.0);
   ortho_projection           = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
   glUniformMatrix4fv(glGetUniformLocation(debug_screen_space_shader, "orthographic"), 1, GL_FALSE, &ortho_projection[0][0]);

   glBindVertexArray(debug_quad.vao);
   glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, instance_count);

   glUseProgram(triangle_shader);

   glm::mat4 model   = glm::mat4(1.0f);
   glm::mat4 proj    = glm::mat4(1.0f);
   glm::mat4 view    = glm::mat4(1.0f);

   model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   model = glm::scale(model, glm::vec3(0.5, 0.5, 0.0));
   proj  = glm::perspective(glm::radians(45.0f), (float)screen_w/(float)screen_h, 0.1f, 100.0f);
   view  = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));

   glUniformMatrix4fv(glGetUniformLocation(triangle_shader, "model"), 1, GL_FALSE, &model[0][0]);
   glUniformMatrix4fv(glGetUniformLocation(triangle_shader, "view"),  1, GL_FALSE, &view[0][0]);
   glUniformMatrix4fv(glGetUniformLocation(triangle_shader, "proj"),  1, GL_FALSE, &proj[0][0]);

   glBindVertexArray(quad.vao);
   glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, instance_count);

   glBindVertexArray(0);
}

function void
gl_swap_framebuffers(SDL_Window *window)
{
   SDL_GL_SwapWindow(window);
}

function void
gl_cleanup()
{
   glDeleteVertexArrays(1, &quad.vao);
   glDeleteBuffers(1, &quad.vbo);
   glDeleteProgram(triangle_shader);
   SDL_GL_DeleteContext(&gl_context);
}