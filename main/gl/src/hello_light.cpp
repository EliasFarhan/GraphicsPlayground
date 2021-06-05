//
// Created by efarhan on 6/5/21.
//

#include "hello_light.h"

#define _USE_MATH_DEFINES
#include <cmath>
namespace gl
{

void HelloLight::Init()
{
    camera_.Init();
    cube_.Init();

    lampProgram_.CreateDefaultProgram("data/shaders/04_hello_light/lamp.vert",
                                      "data/shaders/04_hello_light/lamp.frag");
    litProgram_.CreateDefaultProgram("data/shaders/04_hello_light/light.vert",
                                     "data/shaders/04_hello_light/light.frag");
    glEnable(GL_DEPTH_TEST);
}

void HelloLight::Update(core::seconds dt)
{
    camera_.Update(dt);
    time_ += dt.count();
    lampPos_ = glm::vec3(std::cos(2.0f*M_PI*time_/lampPeriod_), 0.0f,
                         std::sin(2.0f*M_PI*time_/lampPeriod_)) * lampRadius_;

    const auto view = camera_.GetView();
    const auto projection = camera_.GetProjection();
    //Render cube light
    lampProgram_.Bind();
    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3 (0.2f, 0.2f, 0.2f));
    model = glm::translate(model, lampPos_);
    lampProgram_.SetMat4("model", model);
    lampProgram_.SetMat4("view", view);
    lampProgram_.SetMat4("projection", projection);

    lampProgram_.SetVec3("lightColor", lightColor_);
    cube_.Draw();

    //Render center cube
    litProgram_.Bind();
    model = glm::mat4(1.0f);
    litProgram_.SetMat4("model", model);
    litProgram_.SetMat4("view", view);
    litProgram_.SetMat4("projection", projection);
    litProgram_.SetVec3("lightPos", lampPos_);
    litProgram_.SetVec3("viewPos", camera_.position);
    litProgram_.SetVec3("objectColor", objectColor_);
    litProgram_.SetVec3("lightColor", lightColor_);
    litProgram_.SetFloat("ambientStrength", ambientStrength_);
    litProgram_.SetFloat("diffuseStrength", diffuseStrength_);
    litProgram_.SetFloat("specularStrength", specularStrength_);
    litProgram_.SetInt("specularPow", specularPow_);

    const auto inverseTransposeModel = glm::transpose(glm::inverse(model));
    litProgram_.SetMat4("inverseTransposeModel", inverseTransposeModel);
    cube_.Draw();

    auto* window = Engine::GetInstance().GetWindow();
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    camera_.SetAspect(glm::vec2(width, height));
}

void HelloLight::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    litProgram_.Destroy();
    lampProgram_.Destroy();
    cube_.Destroy();
}

void HelloLight::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloLight::DrawImGui()
{
    ImGui::Begin("Light program");
    ImGui::InputFloat("ambientStrength", &ambientStrength_);
    ImGui::InputFloat("diffuseStrength", &diffuseStrength_);
    ImGui::InputFloat("specularStrength", &specularStrength_);
    ImGui::InputInt("specularPow", &specularPow_);
    ImGui::ColorEdit3("Object Color", &objectColor_[0]);
    ImGui::ColorEdit3("Light Color", &lightColor_[0]);
    ImGui::End();
}
}