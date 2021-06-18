#include <algorithm>
#include <array>
#include "log.h"
#include <gl/vertex_array.h>
#include "gl/error.h"
#include "GL/glew.h"
#include <cmath>

#include <vector>
#include <glm/vec3.hpp>

#ifdef TRACY_ENABLE

#include "Tracy.hpp"
#include "TracyOpenGL.hpp"

#endif
namespace gl
{
VertexArray::~VertexArray()
{
    if (vao_)
    {
        core::LogWarning("Vertex Array is not free");
    }
}

void VertexArray::GenerateVao()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(generateVao, "Generate VAO", true);
    TracyGpuNamedZone(generateVaoGpu, "Generate VAO", true);
#endif
    glGenVertexArrays(1, &vao_);
    glCheckError();
}

void VertexArray::FreeVao()
{
    if (vao_ != 0)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(freeVao, "Free VAO", true);
        TracyGpuNamedZone(freeVaoGpu, "Free VAO", true);
#endif
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
        glCheckError();
    }
}

Quad::Quad(glm::vec2 size, glm::vec2 offset) : size_(size), offset_(offset)
{
}

Quad::~Quad()
{
    if (ebo_)
    {
        core::LogWarning("Quad is not free");
    }
}

void Quad::Init()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(quadInit, "Quad Init", true);
    TracyGpuNamedZone(quadInitGpu, "Quad Init", true);
#endif
    const glm::vec2 vertices[4] = {
            glm::vec2(0.5f, 0.5f) * size_ + offset_,  // top right
            glm::vec2(0.5f, -0.5f) * size_ + offset_,  // bottom right
            glm::vec2(-0.5f, -0.5f) * size_ + offset_,  // bottom left
            glm::vec2(-0.5f, 0.5f) * size_ + offset_   // top left
    };

    const glm::vec2 texCoords[4] = {
            glm::vec2(1.0f, 1.0f),      // top right
            glm::vec2(1.0f, 0.0f),   // bottom right
            glm::vec2(0.0f, 0.0f),   // bottom left
            glm::vec2(0.0f, 1.0f),   // bottom left
    };

    constexpr glm::vec3 back(0, 0, -1);
    constexpr glm::vec3 normals[4] = {
            back,
            back,
            back,
            back
    };

    std::array<glm::vec3, 4> tangent{};

    {
        const glm::vec2 edge1(vertices[1] - vertices[0]);
        const glm::vec2 edge2(vertices[2] - vertices[0]);
        const glm::vec2 deltaUV1 = texCoords[1] - texCoords[0];
        const glm::vec2 deltaUV2 = texCoords[2] - texCoords[0];

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent[0].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent[0].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent[0].z = 0.0f;
    }
    std::fill(tangent.begin() + 1, tangent.end(), tangent[0]);

    constexpr unsigned int indices[6] = {
            // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle
    };
    GenerateVao();
    //Initialize the EBO program
    glGenBuffers(4, &vbo_[0]);
    glGenBuffers(1, &ebo_);
    glCheckError();

    glBindVertexArray(vao_);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*) 0);
    glEnableVertexAttribArray(0);
    //bind texture coords data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(1);
    // bind normals data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);
    glEnableVertexAttribArray(2);
    // bind tangent data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), &tangent[0], GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);
    glEnableVertexAttribArray(3);
    //bind EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
    glCheckError();
}

void Quad::Destroy()
{
    FreeVao();
    FreeBuffers();
}

void Quad::Draw()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(drawQuad, "Draw Quad", true);
    TracyGpuNamedZone(drawQuadGpu, "Draw Quad", true);
#endif
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glCheckError();
}

void Quad::FreeBuffers()
{
    if (ebo_ != 0)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(freeBuffers, "Free Buffers", true);
        TracyGpuNamedZone(freeBuffersGpu, "Free Buffers", true);
#endif
        glDeleteBuffers(4, &vbo_[0]);
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
        glCheckError();
    }
}

Cuboid::Cuboid(glm::vec3 size, glm::vec3 offset) : size_(size), offset_(offset)
{
}

Cuboid::~Cuboid()
{
    if (vbo_[0])
    {
        core::LogWarning("Cube is not free");

    }
}

void Cuboid::Init()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(cubeInit, "Cube Init", true);
    TracyGpuNamedZone(cubeInitGpu, "Cube Init", true);
#endif
    glm::vec3 position[36] =
            {
                    //Right face
                    glm::vec3(0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, 0.5f) * size_ + offset_,
                    //Left face                 *size_+offset_
                    glm::vec3(-0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, 0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, -0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, 0.5f, 0.5f) * size_ + offset_,
                    //Top face                  *size_+offset_
                    glm::vec3(-0.5f, 0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, 0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, 0.5f, 0.5f) * size_ + offset_,
                    //Bottom fa                 *size_+offset_
                    glm::vec3(-0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, -0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, -0.5f, -0.5f) * size_ + offset_,
                    //Front fac                 *size_+offset_
                    glm::vec3(-0.5f, -0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, 0.5f, 0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, -0.5f, 0.5f) * size_ + offset_,
                    //Back face
                    glm::vec3(-0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(0.5f, 0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, -0.5f, -0.5f) * size_ + offset_,
                    glm::vec3(-0.5f, 0.5f, -0.5f) * size_ + offset_,
            };
    glm::vec2 texCoords[36] = {
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),

            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),

            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 1.0f),
    };

    glm::vec3 normals[36] =
            {
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f),

                    glm::vec3(-1.0f, 0.0f, 0.0f),
                    glm::vec3(-1.0f, 0.0f, 0.0f),
                    glm::vec3(-1.0f, 0.0f, 0.0f),
                    glm::vec3(-1.0f, 0.0f, 0.0f),
                    glm::vec3(-1.0f, 0.0f, 0.0f),
                    glm::vec3(-1.0f, 0.0f, 0.0f),

                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f),

                    glm::vec3(0.0f, -1.0f, 0.0f),
                    glm::vec3(0.0f, -1.0f, 0.0f),
                    glm::vec3(0.0f, -1.0f, 0.0f),
                    glm::vec3(0.0f, -1.0f, 0.0f),
                    glm::vec3(0.0f, -1.0f, 0.0f),
                    glm::vec3(0.0f, -1.0f, 0.0f),

                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f),

                    glm::vec3(0.0f, 0.0f, -1.0f),
                    glm::vec3(0.0f, 0.0f, -1.0f),
                    glm::vec3(0.0f, 0.0f, -1.0f),
                    glm::vec3(0.0f, 0.0f, -1.0f),
                    glm::vec3(0.0f, 0.0f, -1.0f),
                    glm::vec3(0.0f, 0.0f, -1.0f),
            };

    glm::vec3 tangent[36]{};
    for (int i = 0; i < 36; i += 3)
    {
        const glm::vec3 edge1 = position[i + 1] - position[i];
        const glm::vec3 edge2 = position[i + 2] - position[i];
        const glm::vec2 deltaUV1 = texCoords[i + 1] - texCoords[i];
        const glm::vec2 deltaUV2 = texCoords[i + 2] - texCoords[i];

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent[i].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent[i].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent[i].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent[i + 1] = tangent[i];
        tangent[i + 2] = tangent[i];
    }
    GenerateVao();
    glGenBuffers(4, &vbo_[0]);
    glCheckError();

    glBindVertexArray(vao_);
    // position attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    // normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(2);
    //tangent attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangent), tangent, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    glCheckError();
}

void Cuboid::Destroy()
{
    FreeVao();
    FreeBuffers();
}

void Cuboid::Draw()
{
#ifdef TRACY_ENABLE
    ZoneNamedN(drawCube, "Draw Cube", true);
    TracyGpuNamedZone(drawCubeGpu, "Draw Cube", true);
#endif
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glCheckError();
}

void Cuboid::FreeBuffers()
{
    if (vbo_[0] != 0)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(freeBuffers, "Free Buffers", true);
        TracyGpuNamedZone(freeBuffersGpu, "Free Buffers", true);
#endif
        glDeleteBuffers(4, &vbo_[0]);
        vbo_[0] = 0;
        glCheckError();
    }
}

void Sphere::Init()
{
    glCheckError();
    GenerateVao();

    glGenBuffers(1, &vbo_[0]);
    glGenBuffers(1, &ebo_);

    std::vector<glm::vec3> positions;
    positions.reserve((segment_ + 1) * (segment_ + 1));
    std::vector<glm::vec2> uv;
    uv.reserve((segment_ + 1) * (segment_ + 1));
    std::vector<glm::vec3> normals;
    normals.reserve((segment_ + 1) * (segment_ + 1));
    std::vector<glm::vec3> tangent;
    tangent.resize((segment_ + 1) * (segment_ + 1));
    std::vector<unsigned int> indices;
    indices.reserve((segment_ + 1) * segment_);
    for (unsigned int y = 0; y <= segment_; ++y)
    {
        for (unsigned int x = 0; x <= segment_; ++x)
        {
            float xSegment = static_cast<float>(x) / static_cast<float>(segment_);
            float ySegment = static_cast<float>(y) / static_cast<float>(segment_);
            float xPos = offset_.x + radius_ * std::cos(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);
            float yPos = offset_.y + radius_ * std::cos(ySegment * M_PI);
            float zPos = offset_.z + radius_ * std::sin(xSegment * 2.0f * M_PI) * std::sin(ySegment * M_PI);

            positions.emplace_back(xPos, yPos, zPos);
            uv.emplace_back(xSegment, ySegment);
            normals.emplace_back(xPos, yPos, zPos);
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < segment_; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= segment_; ++x)
            {
                indices.push_back(y * (segment_ + 1) + x);
                indices.push_back((y + 1) * (segment_ + 1) + x);
            }
        }
        else
        {
            for (int x = segment_; x >= 0; --x)
            {
                indices.push_back((y + 1) * (segment_ + 1) + x);
                indices.push_back(y * (segment_ + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    indexCount_ = indices.size();
    for (size_t i = 0; i < indexCount_ - 2; i++)
    {
        const glm::vec3 edge1 = positions[indices[i + 1]] - positions[indices[i]];
        const glm::vec3 edge2 = positions[indices[i + 2]] - positions[indices[i]];
        const glm::vec2 deltaUV1 = uv[indices[i + 1]] - uv[indices[i]];
        const glm::vec2 deltaUV2 = uv[indices[i + 2]] - uv[indices[i]];

        const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        tangent[indices[i]].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent[indices[i]].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent[indices[i]].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    }

    std::vector<float> data;
    data.reserve(
            positions.size() * sizeof(glm::vec3) + uv.size() * sizeof(glm::vec2) + normals.size() * sizeof(glm::vec3));
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (!uv.empty())
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if (!normals.empty())
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
        if (!tangent.empty())
        {
            data.push_back(tangent[i].x);
            data.push_back(tangent[i].y);
            data.push_back(tangent[i].z);
        }
    }
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    const auto stride = (3 + 2 + 3 + 3) * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*) (5 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*) (8 * sizeof(float)));
    glBindVertexArray(0);
    glCheckError();
}

void Sphere::Destroy()
{
    FreeVao();
    FreeBuffers();
}

void Sphere::Draw()
{
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount_, GL_UNSIGNED_INT, 0);
}

Sphere::Sphere(float radius, glm::vec3 offset, std::size_t segment) : radius_(radius), segment_(segment),
                                                                      offset_(offset)
{

}

Sphere::~Sphere()
{
    if (vao_ || vbo_[0] || ebo_)
    {
        core::LogWarning("Sphere is not free");
    }
}

void Sphere::FreeBuffers()
{
    if (ebo_)
    {
        glDeleteBuffers(1, &ebo_);
        glDeleteBuffers(4, &vbo_[0]);
        ebo_ = 0;
        std::ranges::fill(vbo_, 0);
    }
}
}
