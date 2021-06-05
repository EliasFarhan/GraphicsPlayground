#pragma once

#include "gl/mesh.h"
#include <assimp/scene.h>

namespace gl
{

class Model
{
public:
    Model() = default;
    ~Model();
    void LoadModel(std::string_view path);
    void Draw(ShaderProgram& shader);
    void Destroy();
private:
    std::vector<Mesh> meshes_;
    std::string directory_;
    std::vector<std::size_t> textureHashes_;
    std::vector<Texture> textures_;

    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Mesh::Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType type, std::string_view typeName);
};

}