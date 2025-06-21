#ifndef MARSHMALLOW_DRAW_H

#include "marshmallow_base.h"

#include <string>
#include <vector>

// @Temporary: These may be moved to draw since these are draw related data structures

// Version 2 of vertex
typedef struct Vertex {
   V3 pos;
   V3 normal;
   V2 texcoord;
   V3 tangent;
   V3 bitangent;
} Vertex;

typedef struct Texture {
   u32 id;
   std::string type;
   std::string path;
   const char *name;
} Texture;

typedef struct Mesh {
   std::vector<Vertex> vertices;
   std::vector<u32> indices;
   std::vector<Texture> textures;
   //std::vector<unsigned int> textures;
   // @Incomplete: These are stored as Marshmallow_GL Buffer Objects
   unsigned int VBO, VAO, EBO;
   // BufferObject buffer_data;
} Mesh;

typedef struct Model {
   std::vector<Texture> textures;
   std::vector<Mesh> meshes;
   std::string dir;
   //const char* dir;
   bool gamma;
} Model;

function void draw_pixel(GameBackbuffer *buffer, s32 x, s32 y, u32 color);
function void test_gradient(GameBackbuffer *buffer, s32 x_offset, s32 y_offset);
function void debug_draw_rect(GameBackbuffer *buffer, V2S p0, V2S p1, f32 r, f32 g, f32 b, f32 a);
function void debug_draw_rectf32(GameBackbuffer *buffer, V2 p0, V2 p1, f32 r, f32 g, f32 b, f32 a);

#define MARSHMALLOW_DRAW_H
#endif
