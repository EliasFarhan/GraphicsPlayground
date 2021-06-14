#pragma once

#include <array>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace gl
{

class VertexArray
{
public:
    virtual ~VertexArray();

    virtual void Init() = 0;

    virtual void Destroy() = 0;

    virtual void Draw() = 0;

    [[nodiscard]] unsigned int GetVao() const
    { return vao_; }

protected:
    void GenerateVao();

    void FreeVao();

    unsigned int vao_ = 0;
};

class Quad final : public VertexArray
{
public:
    Quad(glm::vec2 size, glm::vec2 offset);

    ~Quad() override;

    void Init() override;

    void Destroy() override;

    void Draw() override;

protected:
    void FreeBuffers();

    glm::vec2 size_;
    glm::vec2 offset_;
    std::array<unsigned int, 4> vbo_{};
    unsigned int ebo_ = 0;
};

class Cuboid final : public VertexArray
{
public:
    Cuboid(glm::vec3 size, glm::vec3 offset);

    ~Cuboid() override;

    void Init() override;

    void Destroy() override;

    void Draw() override;

protected:
    void FreeBuffers();

    std::array<unsigned int, 4> vbo_{};
    glm::vec3 size_;
    glm::vec3 offset_;
};

class Sphere final : public VertexArray
{
public:
    Sphere(float radius, glm::vec3 offset, std::size_t segment=64);

    ~Sphere() override;

    void Init() override;

    void Destroy() override;

    void Draw() override;

private:
    void FreeBuffers();

    std::array<unsigned int, 4> vbo_{};
    unsigned int ebo_ = 0;
    glm::vec3 offset_;
    float radius_ = 0.0f;
    std::size_t segment_ = 0;
    std::size_t indexCount_ = 0;
};

}
