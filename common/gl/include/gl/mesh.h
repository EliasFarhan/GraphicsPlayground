#pragma once


#include <vector>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "gl/shader.h"
#include "gl/vertex_array.h"
#include "gl/texture.h"

namespace gl
{

class Mesh
{
public:
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct Texture
    {
        unsigned int textureName;
        std::string type;
    };

    Mesh() = default;

    ~Mesh();

    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<Texture>&& textures);

    Mesh(const Mesh& other) = delete;

    Mesh(Mesh&& other) noexcept = default;

    Mesh& operator=(const Mesh& other) = delete;

    Mesh& operator=(Mesh&& other) noexcept = default;

    void SetupMesh();

    void Draw(ShaderProgram& shader);

    void Destroy();

private:
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
    std::vector<Texture> textures_;

    unsigned vao_ = 0, vbo_ = 0, ebo_ = 0;
};

}