

#include "imgui.h"
#include "hello_triangle.h"

namespace gl
{


    void HelloTriangle::Init()
    {
        basicTriangleProgram_.shaderProgram.CreateDefaultProgram(
            "data/shaders/01_hello_triangle/triangle.vert",
            "data/shaders/01_hello_triangle/triangle.frag");
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
            "data/shaders/01_hello_triangle/quad.vert",
            "data/shaders/01_hello_triangle/quad.frag"
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
            "Triangle\0Quad\0"))
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
