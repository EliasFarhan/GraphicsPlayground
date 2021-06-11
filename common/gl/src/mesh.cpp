#include "gl/mesh.h"
#include "fmt/core.h"

#include <GL/glew.h>
#ifdef TRACY_ENABLE

#include "Tracy.hpp"
#include "TracyOpenGL.hpp"

#endif
namespace gl
{

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<Texture>&& textures) :
        vertices_(std::move(vertices)), indices_(std::move(indices)), textures_(std::move(textures))
{

}

Mesh::Mesh(Mesh&& other) noexcept
{
    vertices_ = std::move(other.vertices_);
    indices_ = std::move(other.indices_);
    textures_ = std::move(other.textures_);
    std::swap(vao_, other.vao_);
    std::swap(vbo_, other.vbo_);
    std::swap(ebo_, other.ebo_);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    vertices_ = std::move(other.vertices_);
    indices_ = std::move(other.indices_);
    textures_ = std::move(other.textures_);
    std::swap(vao_, other.vao_);
    std::swap(vbo_, other.vbo_);
    std::swap(ebo_, other.ebo_);
    return *this;
}

void Mesh::SetupMesh()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Setup Mesh");
#endif
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glCheckError();
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);



    glCheckError();
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) nullptr);

    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords));

    // vertex normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int),
                 indices_.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
    glCheckError();
}

void Mesh::Draw(ShaderProgram& shader)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Draw Mesh");
#endif
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for (unsigned int i = 0; i < textures_.size(); i++)
    {
        if(textures_[i].textureName == 0)
        {
            core::LogWarning("Invalid Texture in Mesh");
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        glCheckError();
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures_[i].type;
        if (name == "texture_diffuse")
        {
            number = fmt::format("{}", diffuseNr);
            diffuseNr++;
        } else if (name == "texture_specular")
        {
            number = fmt::format("{}", specularNr);
            specularNr++;
        }
        const auto uniformName = fmt::format("{}{}", name, number);
        shader.SetInt(uniformName.c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures_[i].textureName);
        glCheckError();
    }
    glActiveTexture(GL_TEXTURE0);
    glCheckError();

    // draw mesh
    glBindVertexArray(vao_);
    glCheckError();
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, nullptr);
    glCheckError();
    glBindVertexArray(0);
    glCheckError();
}

Mesh::~Mesh()
{
    if(vao_)
    {
        core::LogWarning("Mesh is not free");
    }
}

void Mesh::Destroy()
{
    if (vao_ != 0)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(textureDestroy, "Mesh Destroy", true);
        TracyGpuNamedZone(textureDestroyGpu, "Mesh Destroy", true);
#endif
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
        glCheckError();
        vao_ = 0;
    }
}

}