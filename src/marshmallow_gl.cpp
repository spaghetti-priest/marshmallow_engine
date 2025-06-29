#include <string>
#include <stddef.h>

// Helpful article on text rendeirng:
// http://arkanis.de/weblog/2023-08-14-simple-good-quality-subpixel-text-rendering-in-opengl-with-stb-truetype-and-dual-source-blending
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

typedef struct MeshVertex {
   V3 pos;
   V4 color;
   V2 texcoord;
} MeshVertex;

SDL_GLContext gl_context;

Shader triangle_shader;
Shader debug_screen_space_shader;
Shader text_shader;
Shader scene_shader;
Shader cube_shader;

global_variable Model cube_gltf{};

// u32 quad_instance_vbo = 0;

// @Hardcoded: Hardset and default camera position for now
glm::vec3 camera_pos       = glm::vec3(8.0f, 9.0f, 8.0f);
glm::vec3 camera_front     = glm::vec3(0.0f, -0.5f, -0.8f);
glm::vec3 camera_up        = glm::vec3(0.0f, 1.0f, 0.0f);

// glm::vec3 camera_dir       = glm::normalize(start_camera_pos - camera_target);
// glm::vec3 camera_right     = glm::normalize(glm::cross(up, camera_dir));
// glm::vec3 camera_up        = glm::cross(camera_dir, camera_right);


// static bool
// load_gltf_model (const char *filename)
// {
//    tinygltf::TinyGLTF loader;
//    tinygltf::Model model;
//    std::string warn, err;
//    bool r = false;

//    bool loaded = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
//    if (!warn.empty()) {
//       printf("[TINYGLTF_WARNING]: %s", warn.c_str());
//    }

//    if (!err.empty()) {
//       printf("[TINYGLTF_ERROR]: %s", err.c_str());
//    }

//    if(!loaded) {
//       printf("[ERROR]: Failed to load GLTF model %s", filename);
//    } else {
//       printf("GLTF model %s loaded", filename);
//    }

//    return r;
// }

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
   const char *severity_level = "";
   switch(severity)
   {
      case GL_DEBUG_SEVERITY_MEDIUM:
      {
         severity_level = "SEVERITY_LEVEL_MEDIUM";
      } break;
      case GL_DEBUG_SEVERITY_HIGH:
      {
         severity_level = "SEVERITY_LEVEL_HIGH";
      } break;
   };

   if ((severity == GL_DEBUG_SEVERITY_MEDIUM) || (severity == GL_DEBUG_SEVERITY_HIGH)) {
      fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = %s, message = %s\n\n",
            ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity_level, message );
   }
}

function GLuint
compile_shader (const char *filename, GLenum type)
{
   int length, success;
   char error_buf[1024];
   GLenum error_enum;

   MarshFile shader_file   = debug_read_null_terminated_file(filename);
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

function Shader
gl_create_shader_program (const char *vertex_path, const char *frag_path)
{
   Shader new_shader;
   int success;
   char error_buf[1024];

   u32 vertex_shader    = compile_shader(vertex_path, GL_VERTEX_SHADER);
   u32 frag_shader      = compile_shader(frag_path, GL_FRAGMENT_SHADER);
   u32 shader_program   = glCreateProgram();

   const char *shader_name = strrchr(vertex_path, '/') + 1;

   glAttachShader(shader_program, vertex_shader);
   glAttachShader(shader_program, frag_shader);
   glBindFragDataLocation(shader_program, 0, "frag_color");
   glLinkProgram(shader_program);
   glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

   if (!success) {
     glGetProgramInfoLog(shader_program, 1024, NULL, error_buf);
     fprintf(stderr, "[OpenGL]: [ERROR]: Failed to compile shader program: [%s] \n%s \n", shader_name, error_buf);
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

   new_shader.id = shader_program;
   return new_shader;
}

function u32
gl_upload_texture_from_file (u8 *texture_data, int width, int height, int components, bool enable_gamma)
{
   unsigned int texture_id;
   glGenTextures(1, &texture_id);

   GLenum format;
   if      (components == 1)   format = GL_RED;
   else if (components == 3)   format = GL_RGB;
   else if (components == 4)   format = GL_RGBA;
   assert(format != NULL);

   glBindTexture(GL_TEXTURE_2D, texture_id);
   glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texture_data);
   glGenerateMipmap(GL_TEXTURE_2D);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   return texture_id;
}

inline void
gl_set_uniform_v2 (u32 ID, const char *name, const glm::vec2 &value)
{
   glUniform2fv(glGetUniformLocation(ID, name), 1, &value[0]);
}

inline void
gl_set_uniform_mat4 (GLuint program, const char *name, glm::mat4 value)
{
   // @Incomplete: This does not work. Fix!!
   glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &value[0][0]);
   GLenum err = glGetError();
}

inline void
gl_set_vp (Shader *shader)
{
   glUseProgram(shader->id);
   glm::mat4 view    = glm::mat4(1.0f);
   glm::mat4 proj    = glm::mat4(1.0f);

   view  = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
   proj  = glm::perspective(glm::radians(45.0f), (float)screen_w/(float)screen_h, 0.1f, 100.0f);

   glUniformMatrix4fv(glGetUniformLocation(shader->id, "view"),  1, GL_FALSE, &view[0][0]);
   glUniformMatrix4fv(glGetUniformLocation(shader->id, "proj"),  1, GL_FALSE, &proj[0][0]);
}

inline void
gl_set_mvp (Shader *shader, float degrees)
{
   glUseProgram(shader->id);
   glm::mat4 model   = glm::mat4(1.0f);
   glm::mat4 view    = glm::mat4(1.0f);
   glm::mat4 proj    = glm::mat4(1.0f);

   model = glm::rotate(model, glm::radians(degrees), glm::vec3(1.0f, 0.0f, 0.0f));
   view  = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
   proj  = glm::perspective(glm::radians(45.0f), (float)screen_w/(float)screen_h, 0.1f, 100.0f);

   glUniformMatrix4fv(glGetUniformLocation(shader->id, "model"), 1, GL_FALSE, &model[0][0]);
   glUniformMatrix4fv(glGetUniformLocation(shader->id, "view"),  1, GL_FALSE, &view[0][0]);
   glUniformMatrix4fv(glGetUniformLocation(shader->id, "proj"),  1, GL_FALSE, &proj[0][0]);
}

inline void
gl_set_scale (Shader *shader, glm::vec3 scale)
{
   glUseProgram(shader->id);
   glm::mat4 model   = glm::mat4(1.0f);
   model             = glm::scale(model, scale);
   glUniformMatrix4fv(glGetUniformLocation(shader->id, "model"), 1, GL_FALSE, &model[0][0]);
}

/***************************************************************************************
   @Todo: The rendering pipeline strategem
   So the rendering pipeline will eventually be like this where we have a collection of
   buffer objects that specify meshes and primitives. The objects are collated in a list
   of render commands can of like Dx12's command list.
   - Render Quad count: 2
   - Render Mesh count: 1
   - Render Cube count: 4
   ...etc
   All of this is the frame setup
****************************************************************************************/
function void
gl_initialize_mesh_buffers (Mesh *mesh)
{
   // BufferObject mesh_data{};
   printf("Initializing buffers\n");
   glGenVertexArrays(1, &mesh->VAO);
   glGenBuffers(1, &mesh->VBO);
   glGenBuffers(1, &mesh->EBO);

   glBindVertexArray(mesh->VAO);

   glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
   glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);
   glEnableVertexAttribArray(3);
   glEnableVertexAttribArray(4);
   // glEnableVertexAttribArray(6);
   // glEnableVertexAttribArray(5);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
   glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
   glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
   // glVertexAttribIPointer(5, 4, GL_INT,            sizeof(Vertex), (void*)offsetof(Vertex, bone_id));
   // glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weight));

   glBindVertexArray(0);

   // return mesh_data;
}

function void
gl_draw_model (Model *model)
{
   glUseProgram(scene_shader.id);

   glm::mat4 model_space = glm::mat4(1.0f);
   glm::mat4 proj    = glm::mat4(1.0f);
   glm::mat4 view    = glm::mat4(1.0f);

   model_space = glm::translate(model_space, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
   model_space = glm::scale(model_space, glm::vec3(1.0f, 1.0f, 1.0f)); // it's a bit too big for our scene, so scale it down
   proj        = glm::perspective(glm::radians(45.0f), (float)screen_w/(float)screen_h, 0.1f, 100.0f);
   view        = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

   glUniformMatrix4fv(glGetUniformLocation(scene_shader.id, "model"), 1, GL_FALSE, &model_space[0][0]);
   glUniformMatrix4fv(glGetUniformLocation(scene_shader.id, "view"),  1, GL_FALSE, &view[0][0]);
   glUniformMatrix4fv(glGetUniformLocation(scene_shader.id, "proj"),  1, GL_FALSE, &proj[0][0]);

   for (u16 i = 0; i < model->meshes.size(); ++i) {
      Mesh *mesh = &model->meshes[i];

      unsigned int diffuse    = 1;
      unsigned int specular   = 1;
      unsigned int normal     = 1;
      unsigned int height     = 1;

      for (u16 i = 0; i < mesh->textures.size(); ++i) {
         glActiveTexture(GL_TEXTURE0 + i);
         std::string number;
         std::string name = mesh->textures[i].type;

         //@Speed: This could benifit from a hash table
         if      (name == "texture_diffuse")  number = std::to_string(diffuse++);
         else if (name == "texture_specular") number = std::to_string(specular++);
         else if (name == "texture_normal")   number = std::to_string(normal++);
         else if (name == "texture_height")   number = std::to_string(height++);
         glUniform1i(glGetUniformLocation(scene_shader.id, (name + number).c_str()), i);
         glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
      }

      glBindVertexArray(mesh->VAO);
      glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);

      glActiveTexture(GL_TEXTURE0);
   }
}

u32 quad_instance_vbo = 0;
// BufferObject *quad;
BufferObject quad_instance;
BufferObject debug_quad;
u32 quad_count = 0;

// @Note: The V2S vertexes are there to ensure that the user code operates on an integer based coordinate system
// but this may be primer for removal who knows....
function void
gl_push_quad_immediate (V2S min, V2S max, V4 color)
{
   BufferObject object{};

   float minx = (float)min.x;
   float miny = (float)min.y;
   float maxx = (float)(min.x + max.x);
   float maxy = (float)(min.y + max.y);

   const u32 vertex_count = 4;
   MeshVertex vertices[vertex_count];
   vertices[0] = {{maxx, maxy, 0},  color}; // tr
   vertices[1] = {{maxx, miny, 0},  color}; // br
   vertices[2] = {{minx, miny, 0},  color}; // bl
   vertices[3] = {{minx, maxy, 0},  color}; // bl

   glGenVertexArrays(1, &object.vao);
   glBindVertexArray(object.vao);

   glGenBuffers(1, &object.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertex_count, vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &object.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, pos));
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, color));

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   // quad[++quad_count] = object;
}

function void
gl_push_quad_instanced (V2S min, V2S max, V4 color, V2S *translations, u32 instance_count)
{
   BufferObject object{};

   float minx = (float)min.x;
   float miny = (float)min.y;
   float maxx = (float)max.x;
   float maxy = (float)max.y;

   const u32 vertex_count = 4;
   MeshVertex vertices[vertex_count];
   vertices[0] = {{maxx, maxy, 0},  color}; // tr
   vertices[1] = {{maxx, miny, 0},  color}; // br
   vertices[2] = {{minx, miny, 0},  color}; // bl
   vertices[3] = {{minx, maxy, 0},  color}; // tl

   glGenBuffers(1, &object.instance_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, object.instance_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(V2S) * instance_count, &translations[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   object.instance_count = instance_count;

   glGenVertexArrays(1, &object.vao);
   glBindVertexArray(object.vao);

   glGenBuffers(1, &object.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertex_count, vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &object.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, pos));
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, color));

   glBindBuffer(GL_ARRAY_BUFFER, object.instance_vbo);
   glVertexAttribPointer(2, 2, GL_INT, GL_FALSE, sizeof(V2S), (void*)0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   quad_instance = object;
   // return object;
}

function void
gl_draw_quad ()
{
   // glBindVertexArray(vertex_array_object);
   // glDrawElementsInstanced(GL_TRIANGLES, 0, 4, indices, instance_count);
   // glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instance_count);
   // for (u32 i = 0; i <= quad_count; ++i) {
   //    glBindVertexArray(quad[i].vao);
   //    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
   // }
   // glBindVertexArray(0);

}

BufferObject cube_mesh{};
const u32 cube_instance_count = 1;
V3S cube_translations[cube_instance_count] = {v3s(8.0f, 2.0f, -4.0f)};//,
                                              // v3s(4.0f, 2.2f, -4.5f),
                                              // v3s(12.0f, 2.2f, -4.5f)};

// @Note: Despite the name. This function can draw multiple cubes. It should
// be called gl_push_cube_instanced for preciseness
function void
gl_push_cube (V3S *positions, u32 instance_count, V4 color)
{
   const u32 debug_vertex_count_cube = 36;
   MeshVertex debug_cube[debug_vertex_count_cube];
   // back face ?
   debug_cube[0] = {{-1.0f, -1.0f, -1.0f},  color}; // bottom-left
   debug_cube[1] = {{1.0f,  1.0f, -1.0f},  color}; // top-right
   debug_cube[2] = {{1.0f, -1.0f, -1.0f},  color}; // bottom-right
   debug_cube[3] = {{1.0f,  1.0f, -1.0f},  color}; // top-right
   debug_cube[4] = {{-1.0f, -1.0f, -1.0f},  color}; // bottom-left
   debug_cube[5] = {{-1.0f,  1.0f, -1.0f},  color}; // top-left
   // front face
   debug_cube[6] = {{-1.0f, -1.0f,  1.0f},  color}; // bottom-left
   debug_cube[7] = {{1.0f, -1.0f,  1.0f},  color}; // bottom-right
   debug_cube[8] = {{1.0f,  1.0f,  1.0f},  color}; // top-right
   debug_cube[9] = {{1.0f,  1.0f,  1.0f},  color}; // top-right
   debug_cube[10] = {{-1.0f,  1.0f,  1.0f},  color}; // top-left
   debug_cube[11] = {{-1.0f, -1.0f,  1.0f},  color}; // bottom-left
   // left face
   debug_cube[12] = {{-1.0f,  1.0f,  1.0f}, color}; // top-right
   debug_cube[13] = {{-1.0f,  1.0f, -1.0f}, color}; // top-left
   debug_cube[14] = {{-1.0f, -1.0f, -1.0f}, color}; // bottom-left
   debug_cube[15] = {{-1.0f, -1.0f, -1.0f}, color}; // bottom-left
   debug_cube[16] = {{-1.0f, -1.0f,  1.0f}, color}; // bottom-right
   debug_cube[17] = {{-1.0f,  1.0f,  1.0f}, color}; // top-right
   // right face
   debug_cube[18] = {{1.0f,  1.0f,  1.0f}, color}; // top-left
   debug_cube[19] = {{1.0f, -1.0f, -1.0f}, color}; // bottom-right
   debug_cube[20] = {{1.0f,  1.0f, -1.0f}, color}; // top-right
   debug_cube[21] = {{1.0f, -1.0f, -1.0f}, color}; // bottom-right
   debug_cube[22] = {{1.0f,  1.0f,  1.0f}, color}; // top-left
   debug_cube[23] = {{1.0f, -1.0f,  1.0f}, color}; // bottom-left
   // bottom face
   debug_cube[24] = {{-1.0f, -1.0f, -1.0f},  color}; // top-right
   debug_cube[25] = {{1.0f, -1.0f, -1.0f},  color}; // top-left
   debug_cube[26] = {{1.0f, -1.0f,  1.0f},  color}; // bottom-left
   debug_cube[27] = {{1.0f, -1.0f,  1.0f},  color}; // bottom-left
   debug_cube[28] = {{-1.0f, -1.0f,  1.0f},  color}; // bottom-right
   debug_cube[29] = {{-1.0f, -1.0f, -1.0f},  color}; // top-right
   // top face
   debug_cube[30] = {{-1.0f,  1.0f, -1.0f}, color}; // top-left
   debug_cube[31] = {{1.0f,  1.0f , 1.0f},  color}; // bottom-right
   debug_cube[32] = {{1.0f,  1.0f, -1.0f},  color}; // top-right
   debug_cube[33] = {{1.0f,  1.0f,  1.0f},  color}; // bottom-right
   debug_cube[34] = {{-1.0f,  1.0f, -1.0f}, color}; // top-left
   debug_cube[35] = {{-1.0f,  1.0f,  1.0f}, color};  // bottom-left

   // @Incomplete: This mesh instance VBO is only for position
   // We might be able to implement different colors here aswell
   glGenBuffers(1, &cube_mesh.instance_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, cube_mesh.instance_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(V3) * instance_count, &positions[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   glGenVertexArrays(1, &cube_mesh.vao);
   glGenBuffers(1, &cube_mesh.vbo);
   glBindVertexArray(cube_mesh.vao);

   glBindBuffer(GL_ARRAY_BUFFER, cube_mesh.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * debug_vertex_count_cube, debug_cube, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0); // position
   glEnableVertexAttribArray(1); // colors
   glEnableVertexAttribArray(2); // instance

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, pos));
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, color));

   glBindBuffer(GL_ARRAY_BUFFER, cube_mesh.instance_vbo);
   glVertexAttribPointer(2, 3, GL_INT, GL_FALSE, sizeof(V3), (void*)0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   return;
}

function void
gl_draw_cube (Shader *shader)
{
   glUseProgram(shader->id);
   gl_set_mvp(shader, 0.0f);

   glBindVertexArray(cube_mesh.vao);
   glDrawArraysInstanced(GL_TRIANGLES, 0, 36, cube_instance_count);
   // glDrawArrays(GL_TRIANGLES, 0 , 36);
   glBindVertexArray(0);
}

BufferObject text_obj;
GLuint font_texture;

function void
gl_init_font_texture (unsigned char *temp_bitmap, int font_w, int font_h)
{
   glGenTextures(1, &font_texture);
   glBindTexture(GL_TEXTURE_2D, font_texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font_w, font_h, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glGenVertexArrays(1, &text_obj.vao);
   glBindVertexArray(text_obj.vao);

   glGenBuffers(1, &text_obj.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, text_obj.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 5, NULL, GL_STATIC_DRAW);

   glGenBuffers(1, &text_obj.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, text_obj.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, pos));
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, texcoord));

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

}


// Make sure this is being rasterized under orthographic projection
/*
function void
marshmallow_print(char *text, float x, float y, V4 color, stbtt_bakedchar *ascii_table)
{
   glUseProgram(text_shader);
   glm::mat4 ortho_projection = glm::mat4(1.0);
   glm::mat4 ortho_model      = glm::mat4(1.0);
   ortho_projection           = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
   glUniformMatrix4fv(glGetUniformLocation(text_shader, "orthographic"), 1, GL_FALSE, &ortho_projection[0][0]);
   glUniform3f(glGetUniformLocation(text_shader, "text_color"), color.x, color.y, color.z);
   glActiveTexture(GL_TEXTURE0);
   glBindVertexArray(text_obj.vao);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glGenTextures(1, &font_texture);
   glBindTexture(GL_TEXTURE_2D, font_texture);

   while(*text) {
      if (*text >= 32 && *text < 128) {
         stbtt_aligned_quad q;
         stbtt_GetBakedQuad(ascii_table, 512,512, *text - 32, &x, &y, &q, 1);//1=opengl & d3d10+,0=d3d9
         float texcoord[4][5] = {
            {q.x1, q.y0, 0.0f, q.s1, q.t0}, // tr
            {q.x1, q.y1, 0.0f, q.s1, q.t1}, // br
            {q.x0, q.y1, 0.0f, q.s0, q.t1}, // bl
            {q.x0, q.y0, 0.0f, q.s0, q.t0} // tl
         };

        glBindTexture(GL_TEXTURE_2D, font_texture);
        glBindBuffer(GL_ARRAY_BUFFER, text_obj.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(texcoord), texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
      }
      ++text;
   }
   glBindVertexArray(0);
   glBindTexture(GL_TEXTURE_2D, 0);
   glUseProgram(0);
}
*/

function bool
init_sdl_opengl (SDL_Window *window, u32 screen_w, u32 screen_h)
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

   glDebugMessageCallback(MessageCallback, 0);

   printf("Vendor:   %s\n", glGetString(GL_VENDOR));
   printf("Renderer: %s\n", glGetString(GL_RENDERER));
   printf("Version:  %s\n", glGetString(GL_VERSION));

   if (SDL_GL_SetSwapInterval(0) != 0) {
      printf("[ERROR]: Failed to get GL_SwapInterval {%s}\n", SDL_GetError());
      return 0;
   };

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MULTISAMPLE);

   unsigned int indices[index_count] =
   {
      0, 2, 1,
      0, 1, 3
   };

   // @Todo: When the backbuffer is resized the bb should retrieve
   // the new screen w and h
   glViewport(0, 0, screen_w, screen_h);
   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);

   // // @Hack: Hardcoded tile_count_x and tile_count_y
   // quad = (BufferObject*)malloc(sizeof(BufferObject) * (12 * 8));
   scene_shader               = gl_create_shader_program("C:/marshmallow/shaders/scene.vert",         "C:/marshmallow/shaders/scene.frag");
   triangle_shader            = gl_create_shader_program("C:/marshmallow/shaders/shader.vert",        "C:/marshmallow/shaders/shader.frag");
   debug_screen_space_shader  = gl_create_shader_program("C:/marshmallow/shaders/debug_shader.vert",  "C:/marshmallow/shaders/debug_shader.frag");
   text_shader                = gl_create_shader_program("C:/marshmallow/shaders/text.vert",          "C:/marshmallow/shaders/text.frag");
   cube_shader                = gl_create_shader_program("C:/marshmallow/shaders/cube.vert",          "C:/marshmallow/shaders/cube.frag");

   const u32 debug_vertex_count = 4;
   MeshVertex debug_vertices[debug_vertex_count];
   V4 gl_color_pink = v4(1.0f, 0.0f, 1.0f, 1.0f);
   V4 gl_color_blue = v4(0.0f, 0.0f, 1.0f, 1.0f);

   debug_vertices[0] = {{1280,  20.0f, 0.0f}, gl_color_pink}; // tr
   debug_vertices[1] = {{1280,  0.0f,  0.0f}, gl_color_pink}; // br
   debug_vertices[2] = {{0.0f,  0.0f,  0.0f}, gl_color_pink}; // bl
   debug_vertices[3] = {{0.0f,  20.0f, 0.0f}, gl_color_pink}; // tl

   glGenVertexArrays(1, &debug_quad.vao);
   glBindVertexArray(debug_quad.vao);

   glGenBuffers(1, &debug_quad.vbo);
   glBindBuffer(GL_ARRAY_BUFFER, debug_quad.vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * debug_vertex_count, debug_vertices, GL_STATIC_DRAW);

   glGenBuffers(1, &debug_quad.ebo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debug_quad.ebo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, pos));
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, color));

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   gl_push_cube(cube_translations, 1, gl_color_blue);
   // debug_quad = push_quad_immediate(debug_vertices, debug_vertex_count);

   return true;
}

function void
gl_begin_frame()
{
   quad_count = 0;
}

float lastx = (float)screen_w / 2.0f;
float lasty = (float)screen_h / 2.0f;
float yaw         = -90.0f;
float pitch       =  0.0f;

function void
gl_render_frame(bool init_mouse, GameInput *input)
{
   GLenum err;
   // @Hardcoded : width and height are hardcoded
   u32 screen_w = 1280;
   u32 screen_h = 720;
   // camera_pos = {}
   #if 1
   // Camera position movement
   {
      const float camera_speed = 0.5f;
      if (input->w_pressed)
         camera_pos += camera_speed * camera_front;
      if (input->s_pressed)
         camera_pos -= camera_speed * camera_front;
      if (input->a_pressed)
         camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
      if (input->d_pressed)
         camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
      if (input->spacebar_pressed)
         camera_pos += glm::vec3(0,.07,0);
      if (input->left_control_pressed)
         camera_pos -= glm::vec3(0,.07,0);
   }
   #endif

   // Camera directional movement
   if (input->right_button_pressed)
   {
      SDL_SetRelativeMouseMode(SDL_TRUE);
      if (init_mouse) {
         // lastx = input->mouse_state.pos.x;
         // lasty = input->mouse_state.pos.y;
         init_mouse = false;
      }

      float xoffset = (float)input->mouse_state.vel.x;
      float yoffset = -(float)input->mouse_state.vel.y;
      // printf("velx = %d || vely = %d\n", input->mouse_state.vel.x, input->mouse_state.vel.y);

      // @Incomplete: sensitivity should be affected by the game state time
      float sensitivity = 0.1f;
      xoffset *= sensitivity;
      yoffset *= sensitivity;

      yaw   += xoffset;
      pitch += yoffset;

      if (pitch > 89.0f)
        pitch = 89.0f;
      if (pitch < -89.0f)
        pitch = -89.0f;

      // @Bug: On initial right mouse click, the y and z of camera front jump to random values. Problem seems to lie in the pitch and its calculation to radians
      camera_front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
      camera_front.y = sin(glm::radians(pitch));
      camera_front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
      camera_front   = glm::normalize(camera_front);
      printf("");
   } else {
      SDL_SetRelativeMouseMode(SDL_FALSE);
   }

   // printf("camera pos x %.3f y %.3f z %.3f\n", camera_pos.x, camera_pos.y, camera_pos.z);
   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(debug_screen_space_shader.id);
   glm::mat4 ortho_projection = glm::mat4(1.0);
   glm::mat4 ortho_model      = glm::mat4(1.0);
   ortho_projection           = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
   glUniformMatrix4fv(glGetUniformLocation(debug_screen_space_shader.id, "orthographic"), 1, GL_FALSE, &ortho_projection[0][0]);

   glBindVertexArray(debug_quad.vao);
   // glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, instance_count);
   glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);

   glUseProgram(triangle_shader.id);

   gl_set_vp(&triangle_shader);
   glm::mat4 model = glm::mat4(1.0f);
   model           = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   model           = glm::scale(model, glm::vec3(0.5, 0.5, 0.0));
   glUniformMatrix4fv(glGetUniformLocation(triangle_shader.id, "model"), 1, GL_FALSE, &model[0][0]);

   glBindVertexArray(quad_instance.vao);
   glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
   glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, quad_instance.instance_count);
   glBindVertexArray(0);

   gl_draw_cube(&cube_shader);

   // gl_draw_model(&cube_gltf);
   // for (u32 i = 0; i <= quad_count; ++i) {
   //    glBindVertexArray(quad[i].vao);
   //    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
   // }
   // glBindVertexArray(quad.vao);
   // glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
   // glDrawElementsInstanced(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0, instance_count);

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
   // for(u32 i = 0; i <= quad_count; ++i) {
   //    glDeleteVertexArrays(1, &quad[i].vao);
   //    glDeleteBuffers(1, &quad[i].vbo);
   // }
   glDeleteProgram(triangle_shader.id);
   SDL_GL_DeleteContext(&gl_context);
}