#include "hello_blending.h"

#include <imgui.h>

namespace gl
{
void HelloBlending::Init()
{
    blendingProgram_.CreateDefaultProgram(
        "data/shaders/09_hello_blending/blending.vert",
        "data/shaders/09_hello_blending/blending.frag");
    plane_.Init();
    cube_.Init();
    windowTexture_.LoadTexture("data/textures/blending_transparent_window.png");
    cubeTexture_.LoadTexture("data/textures/container.jpg");
    whiteTexture_.CreateWhiteTexture();

    camera_.Init();


    glEnable(GL_DEPTH_TEST);
}

void HelloBlending::Update(core::seconds dt)
{
    camera_.Update(dt);

    if (flags_ & ENABLE_BLENDING)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    blendingProgram_.Bind();
    blendingProgram_.SetMat4("view", camera_.GetView());
    blendingProgram_.SetMat4("projection", camera_.GetProjection());
    //Draw floor

    blendingProgram_.SetTexture("texture1", whiteTexture_, 0);
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, -1, 0) * 0.5f);
    model = glm::scale(model, glm::vec3(5.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1,0,0));
    blendingProgram_.SetMat4("model", model);
    plane_.Draw();

    //Draw cube
    blendingProgram_.SetTexture("texture1", cubeTexture_, 0);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
    blendingProgram_.SetMat4("model", model);

    cube_.Draw();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
    blendingProgram_.SetMat4("model", model);
    cube_.Draw();


    //Draw windows
    blendingProgram_.SetTexture("texture1", windowTexture_, 0);
    constexpr std::array windowPositions
    {
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3(1.5f, 0.0f, 0.51f),
        glm::vec3(0.0f, 0.0f, 0.7f),
        glm::vec3(-0.3f, 0.0f, -2.3f),
        glm::vec3(0.5f, 0.0f, -0.6f)
    };
    if (flags_ & ENABLE_SORTING)
    {
        std::array<glm::vec3, 5> sortedWindows = windowPositions;
        std::ranges::sort(sortedWindows,
                          [this](const glm::vec3& v1, const glm::vec3& v2)
                          {
                              const auto d1 = (v1 - camera_.position);
                              const auto d2 = (v2 - camera_.position);
                              return glm::dot(d1, d1) > glm::dot(d2, d2);
                          });
        for (const auto& position : sortedWindows)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            blendingProgram_.SetMat4("model", model);
            plane_.Draw();
        }
    }
    else
    {
        for (const auto& position : windowPositions)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            blendingProgram_.SetMat4("model", model);
            plane_.Draw();
        }
    }
    if (flags_ & ENABLE_BLENDING)
    {
        glDisable(GL_BLEND);
    }
}

void HelloBlending::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    plane_.Destroy();
    cube_.Destroy();
    blendingProgram_.Destroy();
    windowTexture_.Destroy();
    whiteTexture_.Destroy();
    cubeTexture_.Destroy();
}

void HelloBlending::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloBlending::DrawImGui()
{
    ImGui::Begin("Blending Program");
    bool enableBlending = flags_ & ENABLE_BLENDING;
    if (ImGui::Checkbox("Enable Blending", &enableBlending))
    {
        flags_ = enableBlending ? flags_ | ENABLE_BLENDING : flags_ & ~ENABLE_BLENDING;
    }
    bool enableSorting = flags_ & ENABLE_SORTING;
    if (ImGui::Checkbox("Enable Sorting", &enableSorting))
    {
        flags_ = enableSorting ? flags_ | ENABLE_SORTING : flags_ & ~ENABLE_SORTING;
    }
    ImGui::End();
}
}
