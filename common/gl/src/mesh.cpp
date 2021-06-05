#include "gl/mesh.h"
#include "fmt/core.h"

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
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    CheckError(__FILE__, __LINE__);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int),
                 indices_.data(), GL_STATIC_DRAW);

    CheckError(__FILE__, __LINE__);
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) nullptr);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
    CheckError(__FILE__, __LINE__);
}

void Mesh::Draw(ShaderProgram& shader)
{
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
        CheckError(__FILE__, __LINE__);
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
        CheckError(__FILE__, __LINE__);
    }
    glActiveTexture(GL_TEXTURE0);
    CheckError(__FILE__, __LINE__);

    // draw mesh
    glBindVertexArray(vao_);
    CheckError(__FILE__, __LINE__);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    CheckError(__FILE__, __LINE__);
    glBindVertexArray(0);
    CheckError(__FILE__, __LINE__);
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
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
        CheckError(__FILE__, __LINE__);
        vao_ = 0;
    }
}

}