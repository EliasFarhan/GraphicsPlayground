#include <algorithm>
#include "hello_cube.h"
#include "gl/error.h"
#include "imgui.h"
#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#include "TracyOpenGL.hpp"
#endif

namespace gl
{

void HelloCube::Init()
{
    cuboid_.Init();
    cubeTexture_.LoadTexture("data/textures/brickwall.jpg");
    shader_.CreateDefaultProgram("data/shaders/03_hello_rotate_cube/cube.vert",
                                 "data/shaders/03_hello_rotate_cube/cube.frag");
    glEnable(GL_DEPTH_TEST);
    glCheckError();

    positions_ = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)
    };
    std::ranges::fill(quaternions_, glm::identity<glm::quat>());
    const auto windowSize = Engine::GetInstance().GetWindowSize();

    screenSize_ = glm::vec2(windowSize[0], windowSize[1]);

}

void HelloCube::Update(core::seconds dt)
{
#ifdef TRACY_ENABLE
    ZoneScopedN("Cube Sample Loop");
#endif
    time_ += dt.count();

    for(auto& quaternion : quaternions_)
    {
        quaternion = glm::rotate(quaternion, glm::radians(45.0f*dt.count()), glm::vec3(0,0,1));
        quaternion = glm::rotate(quaternion, glm::radians(45.0f*dt.count()), glm::vec3(0,1,0));
    }

    shader_.Bind();
    glm::mat4 view = glm::mat4(1.0f);
    // note that we're translating the scene in the reverse direction of where we want to move
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    shader_.SetMat4("view", view);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), screenSize_.x / screenSize_.y, 0.1f, 100.0f);
    shader_.SetMat4("projection", projection);

    shader_.SetTexture("ourTexture", cubeTexture_,0);
    for (std::size_t i = 0; i < cubeNmb; i++)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, positions_[i]);
        model = model * glm::mat4_cast(quaternions_[i]);
        shader_.SetMat4("model", model);
        cuboid_.Draw();
    }
}

void HelloCube::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    glCheckError();
    shader_.Destroy();
    cuboid_.Destroy();
    cubeTexture_.Destroy();
}

void HelloCube::OnEvent(SDL_Event& event)
{
    if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        screenSize_ = {event.window.data1, event.window.data2};
    }
}

void HelloCube::DrawImGui()
{
}
}
