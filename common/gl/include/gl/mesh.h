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
        glm::vec3 position;
        glm::vec2 texCoords;
        glm::vec3 normal;
        glm::vec3 tangent;
    };

    struct Texture
    {
        unsigned int textureName = 0;
        std::string type;
    };

    Mesh() = default;

    ~Mesh();

    Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices,
         std::vector<Texture>&& textures);

    Mesh(const Mesh& other) = delete;

    Mesh(Mesh&& other) noexcept;

    Mesh& operator=(const Mesh& other) = delete;

    Mesh& operator=(Mesh&& other) noexcept;

    void SetupMesh();

    void Draw(ShaderProgram& shader);

    void Destroy();

    [[nodiscard]] unsigned int GetVao() const
    { return vao_; }

    [[nodiscard]] std::size_t GetIndicesCount() const;

    void BindTextures(ShaderProgram& shader) const;

    [[nodiscard]] glm::vec3 GetMax() const {return maxExtend;}
    [[nodiscard]] glm::vec3 GetMin() const {return minExtend;}

private:
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
    std::vector<Texture> textures_;
    glm::vec3 maxExtend{std::numeric_limits<float>::lowest()}, minExtend{std::numeric_limits<float>::max()};
    unsigned vao_ = 0, vbo_ = 0, ebo_ = 0;
};

}