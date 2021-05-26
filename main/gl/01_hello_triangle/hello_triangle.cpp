#include <SDL_main.h>

#include "gl/engine.h"
#include <GL/glew.h>

#include "imgui.h"
#include "gl/shader.h"
#include "gl/vertex_array.h"

namespace gl
{
    class HelloTriangle : public Program
    {
    public:
        void Init() override;
        void Update(seconds dt) override;
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
    };

    void HelloTriangle::Init()
    {
        basicTriangleProgram_.shaderProgram.CreateDefaultProgram(
            "data/shaders/triangle.vert", 
            "data/shaders/triangle.frag");
        glGenVertexArrays(1, &basicTriangleProgram_.VAO);
        // ..:: Initialization code (done once (unless your object frequently changes)) :: ..
        // 1. bind Vertex Array Object
        glBindVertexArray(basicTriangleProgram_.VAO);
        // 2. copy our vertices array in a buffer for OpenGL to use
        glGenBuffers(1, &basicTriangleProgram_.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, basicTriangleProgram_.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(basicTriangleProgram_.vertices), &basicTriangleProgram_.vertices, GL_STATIC_DRAW);
        // 3. then set our vertex attributes pointers
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        basicQuadProgram_.quad.Init();
        basicQuadProgram_.shaderProgram.CreateDefaultProgram(
            "data/shaders/quad.vert",
            "data/shaders/quad.frag"
        );
        basicQuadProgram_.shaderProgram.Bind();
        basicQuadProgram_.shaderProgram.SetVec3("color", glm::vec3(1.0f));
    }

    void HelloTriangle::Update(seconds dt)
    {
        switch (currentProgram_)
        {
        case ProgramType::Triangle: 
            basicTriangleProgram_.shaderProgram.Bind();
            glBindVertexArray(basicTriangleProgram_.VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            break;
        case ProgramType::Quad: 
            basicQuadProgram_.shaderProgram.Bind();
            basicQuadProgram_.quad.Draw();
            break;
        default: 
            break;
        }
        
    }

    void HelloTriangle::Destroy()
    {
        glDeleteVertexArrays(1, &basicTriangleProgram_.VAO);
        glDeleteBuffers(1, &basicTriangleProgram_.VBO);
        
    }

    void HelloTriangle::OnEvent(SDL_Event& event)
    {
    }

    void HelloTriangle::DrawImGui()
    {
        ImGui::Begin("Hello Triangle");
        int currentProgram = static_cast<int>(currentProgram_);
        if(ImGui::Combo("TriangleProgramCombo", &currentProgram,
            "Triangle\0Quad"))
        {
            currentProgram_ = static_cast<ProgramType>(currentProgram);
        }
        if(ImGui::ColorPicker3("Quad Color", &basicQuadProgram_.color[0]))
        {
            basicQuadProgram_.shaderProgram.SetVec3("color", basicQuadProgram_.color);
        }

        ImGui::End();
    }
}

int main(int argc, char** argv)
{
    core::Filesystem filesystem;
    gl::HelloTriangle program;
    gl::Engine engine(program);
    engine.Run();
    return EXIT_SUCCESS;
}
