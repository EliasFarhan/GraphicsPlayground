//
// Created by efarhan on 6/7/21.
//

#include "hello_cubemaps.h"
#include <imgui.h>

namespace gl
{

void HelloCubemaps::Init()
{
    skyboxCube_.Init();
    skyboxShader_.CreateDefaultProgram(
            "data/shaders/11_hello_cubemaps/skybox.vert",
            "data/shaders/11_hello_cubemaps/skybox.frag");
    skyboxTexture_.LoadCubemap({
                                       "data/textures/skybox/right.jpg",
                                       "data/textures/skybox/left.jpg",
                                       "data/textures/skybox/top.jpg",
                                       "data/textures/skybox/bottom.jpg",
                                       "data/textures/skybox/front.jpg",
                                       "data/textures/skybox/back.jpg",
                               });
    ktxTexture_.LoadTexture("data/textures/skybox/skybox.ktx");

    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");
    modelShader_.CreateDefaultProgram(
            "data/shaders/11_hello_cubemaps/model.vert",
            "data/shaders/11_hello_cubemaps/model.frag");
    modelReflectionShader_.CreateDefaultProgram(
            "data/shaders/11_hello_cubemaps/model.vert",
            "data/shaders/11_hello_cubemaps/model_reflection.frag");
    modelRefractionShader_.CreateDefaultProgram(
            "data/shaders/11_hello_cubemaps/model.vert",
            "data/shaders/11_hello_cubemaps/model_refraction.frag");

    cube_.Init();
    cubeTexture_.LoadTexture("data/textures/container.jpg");

    camera_.Init();

    glEnable(GL_DEPTH_TEST);
}

void HelloCubemaps::Update(core::seconds dt)
{
    camera_.Update(dt);

    const auto view = camera_.GetView();
    const auto projection = camera_.GetProjection();
    //Draw model
    switch (currentRenderMode_)
    {
        case ModelRenderMode::NONE:
        {
            modelShader_.Bind();
            modelShader_.SetMat4("view", view);
            modelShader_.SetMat4("projection", projection);
            auto model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            modelShader_.SetMat4("model", model);
            modelShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));

            model_.Draw(modelShader_);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1,0,0) * 2.0f);
            modelShader_.SetMat4("model", model);
            modelShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
            modelShader_.SetTexture("texture_diffuse1", cubeTexture_, 0);
            cube_.Draw();
            break;
        }
        case ModelRenderMode::REFLECTION:
        {
            modelReflectionShader_.Bind();
            modelReflectionShader_.SetMat4("view", view);
            modelReflectionShader_.SetMat4("projection", projection);
            auto model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            modelReflectionShader_.SetMat4("model", model);
            modelReflectionShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
            modelReflectionShader_.SetVec3("cameraPos", camera_.position);
            modelReflectionShader_.SetFloat("reflectionValue", reflectionValue_);
            modelReflectionShader_.SetTexture("skybox", usingKtxTexture_?ktxTexture_:skyboxTexture_, 2);
            model_.Draw(modelReflectionShader_);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1,0,0) * 2.0f);
            modelReflectionShader_.SetMat4("model", model);
            modelReflectionShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
            modelReflectionShader_.SetTexture("texture_diffuse1", cubeTexture_, 0);
            cube_.Draw();
            break;
        }
        case ModelRenderMode::REFRACTION:
        {
            modelRefractionShader_.Bind();
            modelRefractionShader_.SetMat4("view", view);
            modelRefractionShader_.SetMat4("projection", projection);
            auto model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
            modelRefractionShader_.SetMat4("model", model);
            modelRefractionShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));

            modelRefractionShader_.SetFloat("refractiveIndex", refractiveIndex_);
            modelRefractionShader_.SetFloat("refractionValue", refractionValue_);
            modelRefractionShader_.SetVec3("cameraPos", camera_.position);
            modelRefractionShader_.SetTexture("skybox", usingKtxTexture_?ktxTexture_:skyboxTexture_, 2);
            model_.Draw(modelRefractionShader_);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1,0,0) * 2.0f);
            modelRefractionShader_.SetMat4("model", model);
            modelRefractionShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTexture_.GetName());
            cube_.Draw();
            break;
        }
        default:;
    }

    //Draw skybox
    glDepthFunc(GL_LEQUAL);
    skyboxShader_.Bind();
    skyboxShader_.SetMat4("view", glm::mat4(glm::mat3(view)));
    skyboxShader_.SetMat4("projection", projection);
    skyboxShader_.SetTexture("skybox", usingKtxTexture_?ktxTexture_:skyboxTexture_, 0);
    skyboxCube_.Draw();
    glDepthFunc(GL_LESS);
}

void HelloCubemaps::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    skyboxTexture_.Destroy();
    ktxTexture_.Destroy();
    skyboxShader_.Destroy();
    skyboxCube_.Destroy();

    model_.Destroy();
    modelShader_.Destroy();
    modelRefractionShader_.Destroy();
    modelReflectionShader_.Destroy();
    cubeTexture_.Destroy();
    cube_.Destroy();
}

void HelloCubemaps::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloCubemaps::DrawImGui()
{
    ImGui::Begin("Hello Cubemaps");
    const char* renderModeNames_[static_cast<int>(ModelRenderMode::LENGTH)] =
            {
                    "None",
                    "Reflection",
                    "Refraction"
            };
    int currentItem = static_cast<int>(currentRenderMode_);
    if (ImGui::Combo("Render Mode", &currentItem, renderModeNames_, static_cast<int>(ModelRenderMode::LENGTH)))
    {
        currentRenderMode_ = static_cast<ModelRenderMode>(currentItem);
    }
    switch (currentRenderMode_)
    {
        case ModelRenderMode::REFLECTION:
            ImGui::SliderFloat("Reflection Value", &reflectionValue_, 0.0f, 1.0f);
            break;
        case ModelRenderMode::REFRACTION:
            ImGui::SliderFloat("Refraction Value", &refractionValue_, 0.0f, 1.0f);
            ImGui::SliderFloat("Refractive Index", &refractiveIndex_, 1.0f, 3.0f);
            break;
        default:
            break;
    }
    ImGui::Checkbox("Using KTX", &usingKtxTexture_);
    ImGui::End();
}
}