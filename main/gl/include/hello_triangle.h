#pragma once
#include "gl/engine.h"
#include <GL/glew.h>
#include "gl/shader.h"
#include "gl/vertex_array.h"

namespace gl{
class HelloTriangle : public core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:

    struct BasicTriangleProgram
    {
        ShaderProgram shaderProgram;
        float vertices[9] = {
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                0.0f,  0.5f, 0.0f
        };
        GLuint VBO = 0;
        GLuint VAO = 0;
    };
    BasicTriangleProgram basicTriangleProgram_;

    struct BasicQuadProgram
    {
        ShaderProgram shaderProgram;
        glm::vec4 color{1.0f};
        Quad quad{glm::vec2(1.0f), glm::vec2(0.0f)};
    };
    BasicQuadProgram basicQuadProgram_;

    enum class ProgramType
    {
        Triangle,
        Quad,
        Length
    };
    ProgramType currentProgram_ = ProgramType::Triangle;
};}