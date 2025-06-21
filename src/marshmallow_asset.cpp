// #include "marshmallow_asset.h"
#include "marshmallow_gl.h"

// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"

// @Incomplete @Memory @Speed: This needs to be rewritten eventually to utilize arena allocations rather than std library
function std::vector<Texture>
debug_load_material_textures (Model *model, aiMaterial *materials, aiTextureType type, std::string type_name)
{
   std::vector<Texture> textures;

   for (u16 i = 0; i < materials->GetTextureCount(type); ++i) {
      aiString str;
      materials->GetTexture(type, i, &str);
      bool skip = false;
      for (u16 j = 0; j < model->textures.size(); j++) {
         if (std::strcmp(model->textures[j].path.data(), str.C_Str()) == 0) {
            textures.push_back(model->textures[j]);
            skip = true;
            break;
         }
      }

      if (!skip) {
         int width, height, components;
         std::string filename = model->dir + "\\" + str.C_Str();
         u8 *texture_data = stbi_load(filename.c_str(), &width, &height, &components, 0);

         if (!texture_data) {
            printf("[TEXUTRE ERROR]; Texture %s failed to load\n", str.C_Str());
            stbi_image_free(texture_data);
            break;
         }

         Texture texture;
         texture.id     = gl_upload_texture_from_file(texture_data, width, height, components, model->gamma);
         texture.type   = type_name;
         texture.path   = str.C_Str();
         textures.push_back(texture);
         model->textures.push_back(texture);
#if _DEBUG
         printf("[DEBUG]: Texture %s loaded successfully\n", str.C_Str());
#endif
         stbi_image_free(texture_data);
      }
   }

   return textures;
}

function Mesh
asset_process_mesh_data (Model *model, aiMesh *mesh, const aiScene *scene)
{
   Mesh meshes{};

   std::vector<Vertex>         vertices;
   std::vector<unsigned int>   indices;
   std::vector<Texture>        textures;

   for (u16 i = 0; i < mesh->mNumVertices; ++i) {
      Vertex mesh_vertex{};

      mesh_vertex.pos.x = mesh->mVertices[i].x;
      mesh_vertex.pos.y = mesh->mVertices[i].y;
      mesh_vertex.pos.z = mesh->mVertices[i].z;

      if (mesh->HasNormals()) {
         mesh_vertex.normal.x = mesh->mNormals[i].x;
         mesh_vertex.normal.y = mesh->mNormals[i].y;
         mesh_vertex.normal.z = mesh->mNormals[i].z;
      }

      if(mesh->mTextureCoords[0]) {
         mesh_vertex.texcoord.x = mesh->mTextureCoords[0][i].x;
         mesh_vertex.texcoord.y = mesh->mTextureCoords[0][i].y;

         mesh_vertex.tangent.x = mesh->mTangents[i].x;
         mesh_vertex.tangent.y = mesh->mTangents[i].y;
         mesh_vertex.tangent.z = mesh->mTangents[i].z;

         mesh_vertex.bitangent.x = mesh->mBitangents[i].x;
         mesh_vertex.bitangent.y = mesh->mBitangents[i].y;
         mesh_vertex.bitangent.z = mesh->mBitangents[i].z;
      } else {
         mesh_vertex.texcoord = v2(0.0, 0.0);
      }

      vertices.push_back(mesh_vertex);
   }

   for (u16 i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      for (u16 j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
      }
   }

   aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

   std::vector<Texture> diffuse_maps   = debug_load_material_textures(model, material, aiTextureType_DIFFUSE,  "texture_diffuse");
   std::vector<Texture> normal_maps    = debug_load_material_textures(model, material, aiTextureType_HEIGHT,   "texture_normal");
   std::vector<Texture> specular_maps  = debug_load_material_textures(model, material, aiTextureType_SPECULAR, "texture_specular");
   std::vector<Texture> height_maps    = debug_load_material_textures(model, material, aiTextureType_AMBIENT,  "texture_height");

   textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
   textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
   textures.insert(textures.end(), normal_maps.begin(), normal_maps.end());
   textures.insert(textures.end(), height_maps.begin(), height_maps.end());

   meshes = {vertices, indices, textures};
   gl_initialize_mesh_buffers(&meshes);

   return meshes;
}

function void
asset_process_scene (Arena *arena, Model *model, const aiScene *scene, aiNode *node)
{
   for (u16 i = 0; i < node->mNumMeshes; ++i) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      // aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      model->meshes.push_back(asset_process_mesh_data(model, mesh, scene));
   }

   // @Fix: I do not like the recursive nature of this graph traversal
   for (u16 i = 0; i < node->mNumChildren; ++i) {
      asset_process_scene(arena, model, scene, node->mChildren[i]);
   }
}

function bool
asset_load_gltf_model (Arena *arena, Model *model, const char *filename)
{
   std::string path = filename;
   Assimp::Importer importer;
   const aiScene *scene = importer.ReadFile(path,
                                          aiProcess_Triangulate      |
                                          aiProcess_GenSmoothNormals |
                                          aiProcess_FlipUVs          |
                                          aiProcess_CalcTangentSpace);

   if (!scene                                      ||
       scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE   ||
       !scene->mRootNode)
   {
      const char *error = importer.GetErrorString();
      printf("ERROR: %s\n", error);
      return false;
   }

   // @Robustness: if on windows check if path delimiter is \\ else path delimiter is /
   model->dir = path.substr(0, path.find_last_of('\\')).c_str();
   asset_process_scene(arena, model, scene, scene->mRootNode);

   printf("Model succesfully loaded\n");
   return true;
}
