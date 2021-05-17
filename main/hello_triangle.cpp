#include <SDL_main.h>

#include "engine.h"
#include <GL/glew.h>
#include "gl/shader.h"

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
    };

    void HelloTriangle::Init()
    {
        basicTriangleProgram_.shaderProgram.CreateDefaultProgram(
            "data/shaders/hello_triangle/triangle.vert", 
            "data/shaders/hello_triangle/triangle.frag");
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
    }

    void HelloTriangle::Update(seconds dt)
    {
        basicTriangleProgram_.shaderProgram.Bind();
        glBindVertexArray(basicTriangleProgram_.VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
    }
}

int main(int argc, char** argv)
{
    common::Filesystem filesystem;
    gl::HelloTriangle program;
    gl::Engine engine(program);
    engine.Run();
    return EXIT_SUCCESS;
}
