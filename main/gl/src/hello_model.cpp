#include "hello_model.h"

namespace gl
{

void HelloModel::Init()
{
    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");
    shader_.CreateDefaultProgram("data/shaders/07_hello_model/model.vert",
                                 "data/shaders/07_hello_model/model.frag");
    camera_.Init();
    auto* window = Engine::GetInstance().GetWindow();
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    camera_.SetAspect(glm::vec2(width, height));

    glEnable(GL_DEPTH_TEST);
}

void HelloModel::Update(core::seconds dt)
{
    camera_.Update(dt);

    shader_.Bind();
    shader_.SetMat4("view", camera_.GetView());
    shader_.SetMat4("projection", camera_.GetProjection());

    glm::mat4 model(1.0f);
    model = glm::rotate(model, 180.0f, glm::vec3(0,1,0));
    model = glm::scale(model, glm::vec3 (0.1f, 0.1f, 0.1f));
    shader_.SetMat4("model", model);
    shader_.SetMat4("normalMatrix", glm::transpose(glm::inverse(model)));
    model_.Draw(shader_);
}

void HelloModel::Destroy()
{

    glDisable(GL_DEPTH_TEST);
    model_.Destroy();
    shader_.Destroy();
}

void HelloModel::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloModel::DrawImGui()
{

}
}