#include "gl/mesh.h"
#include "fmt/core.h"
#include "gl/error.h"
#include <log.h>

#include <GL/glew.h>

#ifdef TRACY_ENABLE

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

#endif
namespace gl
{

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices,
           std::vector<Texture>&& textures) :
        vertices_(std::move(vertices)), indices_(std::move(indices)),
        textures_(std::move(textures))
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

    for(const auto& vertex : vertices_)
    {
        if(vertex.position.x > maxExtend.x)
        {
            maxExtend.x = vertex.position.x;
        }
        if(vertex.position.y > maxExtend.y)
        {
            maxExtend.y = vertex.position.y;
        }
        if(vertex.position.z > maxExtend.z)
        {
            maxExtend.z = vertex.position.z;
        }

        if(vertex.position.x < minExtend.x)
        {
            minExtend.x = vertex.position.x;
        }
        if(vertex.position.y < minExtend.y)
        {
            minExtend.y = vertex.position.y;
        }
        if(vertex.position.z < minExtend.z)
        {
            minExtend.z = vertex.position.z;
        }

    }

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glCheckError();
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex),
                 vertices_.data(), GL_STATIC_DRAW);


    glCheckError();
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*) nullptr);

    // vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*) offsetof(Vertex, texCoords));

    // vertex normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*) offsetof(Vertex, normal));

    // vertex normals tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*) offsetof(Vertex, tangent));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices_.size() * sizeof(unsigned int),
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
    BindTextures(shader);

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
    if (vao_)
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
        vao_ = 0;
        glCheckError();
        if (vbo_ != 0)
        {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
            glCheckError();
        }
        if (ebo_ != 0)
        {
            glDeleteBuffers(1, &ebo_);
            ebo_ = 0;
            glCheckError();
        }
    }
}

std::size_t Mesh::GetIndicesCount() const
{
    return indices_.size();
}

void Mesh::BindTextures(ShaderProgram& shader) const
{
#ifdef TRACY_ENABLE
    ZoneNamedN(bindTextures, "Bind Textures",
               true);
    TracyGpuNamedZone(bindTexturesGPU,
                      "Bind Textures", true);
#endif
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    for (unsigned int i = 0; i < textures_.size(); i++)
    {
        if (textures_[i].textureName == 0)
        {
            core::LogWarning("Invalid Texture in Mesh");
            continue;
        }
        glActiveTexture(
                GL_TEXTURE0 + i); // activate proper texture unit before binding
        glCheckError();
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures_[i].type;
        if (name == "texture_diffuse")
        {
            number = fmt::format("{}", diffuseNr);
            diffuseNr++;
        }
        else if (name == "texture_specular")
        {
            number = fmt::format("{}", specularNr);
            specularNr++;
        }
        else if (name == "texture_normal")
        {
            number = fmt::format("{}", normalNr);
            normalNr++;
        }
        const auto uniformName = fmt::format("{}{}", name, number);
        shader.SetInt(uniformName.c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures_[i].textureName);
        glCheckError();
    }
    glActiveTexture(GL_TEXTURE0);
    glCheckError();
}



}