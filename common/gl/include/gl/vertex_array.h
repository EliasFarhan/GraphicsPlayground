#pragma once
#include <array>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
namespace gl
{

    class VertexArray
    {
    public:
        virtual ~VertexArray() = default;
        virtual void Init() = 0;
        virtual void Destroy() = 0;
        [[nodiscard]] unsigned int GetVao() const { return vao_; }
    protected:
        void GenerateVao();
        void FreeVao();
        unsigned int vao_ = 0;
    };

    class Quad : public VertexArray
    {
    public:
        Quad(glm::vec2 size, glm::vec2 offset);
        void Init() override;
        void Destroy() override;
    protected:
        glm::vec2 size_;
        glm::vec2 offset_;
        std::array<unsigned int, 4> vbo_{};
        unsigned int ebo_ = 0;
    };

}
