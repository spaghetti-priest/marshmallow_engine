#ifndef MARSHMALLOW_ASSET_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

// // @Temporary: These may be moved to draw since these are draw related data structures

// // Version 2 of vertex
// typedef struct Vertex {
//    V3 pos;
//    V3 normal;
//    V2 texcoord;
//    V3 tangent;
//    V3 bitangent;
// } Vertex;

// typedef struct Texture {
//    u32 id;
//    std::string type;
//    std::string path;
//    const char *name;
// } Texture;

// typedef struct Mesh {
//    std::vector<Vertex> vertices;
//    std::vector<u32> indices;
//    std::vector<Texture> textures;
//    //std::vector<unsigned int> textures;
//    // @Incomplete: These are stored as Marshmallow_GL Buffer Objects
//    unsigned int VBO, VAO, EBO;
//    // BufferObject buffer_data;
// } Mesh;

// typedef struct Model {
//    std::vector<Texture> textures;
//    std::vector<Mesh> meshes;
//    std::string dir;
//    //const char* dir;
//    bool gamma;
// } Model;

function bool asset_load_gltf_model(Arena *arena, Model *model, const char *filename);

#define MARSHMALLOW_ASSET_H
#endif
