#include "gl/model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "gl/texture.h"
#include <algorithm>

#ifdef TRACY_ENABLE

#include "Tracy.hpp"
#include "TracyOpenGL.hpp"

#endif
namespace gl
{

void Model::LoadModel(std::string_view path)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(cubeInit, "Load Model", true);
#endif
    Assimp::Importer import;
    const aiScene* scene = nullptr;
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(importWithAssimp, "Import With Assimp", true);
#endif
        scene = import.ReadFile(path.data(),
                                aiProcess_Triangulate | aiProcess_FlipUVs |
                                aiProcess_GenNormals |
                                aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
            !scene->mRootNode)
        {
            core::LogError(fmt::format("Assimp: {}", import.GetErrorString()));
            return;
        }
    }
#ifdef TRACY_ENABLE
    ZoneNamedN(ProcessNodes, "Process Nodes", true);
#endif
    directory_ = path.substr(0, path.find_last_of('/'));

    ProcessNode(scene->mRootNode, scene);
    std::for_each(meshes_.begin(), meshes_.end(),
                  [](auto& mesh) { mesh.SetupMesh(); });
}

void Model::Draw(ShaderProgram& shader)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Draw Model");
#endif
    for (auto& mesh : meshes_)
    {
        mesh.Draw(shader);
    }
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(ProcessMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    std::vector<Mesh::Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Mesh::Texture> textures;

    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Mesh::Vertex vertex{};
        // process vertex positions, normals and texture coordinates
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        vector.x = mesh->mTangents[i].x;
        vector.y = mesh->mTangents[i].y;
        vector.z = mesh->mTangents[i].z;
        vertex.tangent = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec{};
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    // process indices
    indices.reserve(mesh->mNumFaces * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process material
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Mesh::Texture> diffuseMaps = LoadMaterialTextures(material,
                                                                      aiTextureType_DIFFUSE,
                                                                      "texture_diffuse");
        textures.insert(textures.end(),
                        std::make_move_iterator(diffuseMaps.begin()),
                        std::make_move_iterator(diffuseMaps.end()));
        std::vector<Mesh::Texture> specularMaps = LoadMaterialTextures(material,
                                                                       aiTextureType_SPECULAR,
                                                                       "texture_specular");
        textures.insert(textures.end(),
                        std::make_move_iterator(specularMaps.begin()),
                        std::make_move_iterator(specularMaps.end()));

        std::vector<Mesh::Texture> normalMaps = LoadMaterialTextures(material,
                                                                     aiTextureType_HEIGHT,
                                                                     "texture_normal");
        textures.insert(textures.end(),
                        std::make_move_iterator(normalMaps.begin()),
                        std::make_move_iterator(normalMaps.end()));

    }
    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<Mesh::Texture>
Model::LoadMaterialTextures(aiMaterial* material, aiTextureType type,
                            std::string_view typeName)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Load Material Textures");
#endif
    std::vector<Mesh::Texture> textures;
    for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
        aiString str;
        material->GetTexture(type, i, &str);
        Mesh::Texture texture;
        const auto texturePath = fmt::format("{}/{}", directory_, str.C_Str());
        const auto textureHash = std::hash<std::string>{}(texturePath);
        const auto it = std::ranges::find(textureHashes_, textureHash);
        if (it == textureHashes_.end())
        {
            textures_.emplace_back();
            auto& newTexture = textures_.back();
            newTexture.LoadTexture(texturePath, 0, true, true, false);
            texture.textureName = newTexture.GetName();
            textureHashes_.push_back(textureHash);
        }
        else
        {
            const auto index = std::distance(textureHashes_.begin(), it);
            texture.textureName = textures_[index].GetName();
        }

        texture.type = typeName;
        textures.push_back(std::move(texture));
    }
    return textures;
}

Model::~Model()
{
    Destroy();
}

void Model::Destroy()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(modelDestroy, "Model Destroy", true);
    TracyGpuNamedZone(modelDestroyGpu, "Model Destroy", true);
#endif
    for (auto& mesh : meshes_)
    {
        mesh.Destroy();
    }

    for (auto& texture : textures_)
    {
        texture.Destroy();
    }
}

const Mesh& Model::GetMesh(std::size_t i) const
{
    return meshes_[i];
}
}