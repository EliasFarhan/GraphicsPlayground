//
// Created by efarhan on 14.06.21.
//

#include <imgui_impl_sdl.h>
#include "hello_normal.h"

#include <functional>

namespace gl
{

void HelloNormal::Init()
{
    diffuseShader_.CreateDefaultProgram("data/shaders/15_hello_normal/model.vert",
                                        "data/shaders/15_hello_normal/model.frag");
    normalShader_.CreateDefaultProgram("data/shaders/15_hello_normal/normal.vert",
                                        "data/shaders/15_hello_normal/normal.frag");
    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");
    diffuseTexture_.LoadTexture("data/textures/brickwall.jpg");
    normalTexture_.LoadTexture("data/textures/brickwall_normal.jpg");

    plane_.Init();
    cube_.Init();
    sphere_.Init();
    camera_.Init();
    camera_.position = glm::vec3(-3,3,3);
    camera_.LookAt(glm::vec3());
    glEnable(GL_DEPTH_TEST);
}

void HelloNormal::Update(core::seconds dt)
{
    dt_ += dt.count();
    lightPos_ = glm::vec3(std::cos(dt_), 1.0f, std::sin(dt_)) * 3.0f;
    camera_.Update(dt);

    const auto draw = [this](NormalFlags flag)
    {
        auto model = glm::mat4(1.0f);
        if (flag == ENABLE_MODEL)
        {
            model = glm::scale(model, glm::vec3(0.1f));
        }
        else
        {
            model = glm::scale(model, glm::vec3(3.0f));
        }
        const auto transposeInverseModel = glm::transpose(glm::inverse(model));
        if (flags_ & ENABLE_NORMAL_MAP)
        {
            normalShader_.Bind();
            normalShader_.SetVec3("viewPos", camera_.position);
            normalShader_.SetMat4("view", camera_.GetView());
            normalShader_.SetMat4("projection", camera_.GetProjection());
            normalShader_.SetVec3("lightPos", lightPos_);
            normalShader_.SetMat4("model", model);

            normalShader_.SetMat4("transposeInverseModel", transposeInverseModel);
            normalShader_.SetInt("enableNormalMap", true);
            if (flag != ENABLE_MODEL)
            {
                normalShader_.SetInt("texture_diffuse1", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, diffuseTexture_.GetName());
                normalShader_.SetInt("texture_normal1", 1);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, normalTexture_.GetName());
            }
        }
        else
        {
            diffuseShader_.Bind();
            diffuseShader_.SetVec3("viewPos", camera_.position);
            diffuseShader_.SetVec3("lightPos", lightPos_);

            diffuseShader_.SetMat4("view", camera_.GetView());
            diffuseShader_.SetMat4("projection", camera_.GetProjection());
            diffuseShader_.SetMat4("model", model);
            diffuseShader_.SetMat4("transposeInverseModel", transposeInverseModel);
            if (flag != ENABLE_MODEL)
            {
                diffuseShader_.SetInt("texture_diffuse1", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, diffuseTexture_.GetName());
            }
        }
        switch (flag)
        {
            case ENABLE_PLANE:
                plane_.Draw();
                break;
            case ENABLE_MODEL:
            {
                model_.Draw(flags_ & ENABLE_NORMAL_MAP ? normalShader_ : diffuseShader_);
                break;
            }
            case ENABLE_CUBE:
                cube_.Draw();
                break;
            case ENABLE_SPHERE:
                sphere_.Draw();
                break;
            default:
                break;
        }
    };
    if (flags_ & ENABLE_MODEL)
    {
        draw(ENABLE_MODEL);
    }
    if (flags_ & ENABLE_CUBE)
    {
        draw(ENABLE_CUBE);
    }
    if (flags_ & ENABLE_PLANE)
    {
        draw(ENABLE_PLANE);
    }
    if (flags_ & ENABLE_SPHERE)
    {
        draw(ENABLE_SPHERE);
    }
}

void HelloNormal::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    model_.Destroy();
    diffuseTexture_.Destroy();
    normalTexture_.Destroy();

    normalShader_.Destroy();
    diffuseShader_.Destroy();

    plane_.Destroy();
    cube_.Destroy();
    sphere_.Destroy();
}

void HelloNormal::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
}

void HelloNormal::DrawImGui()
{
    ImGui::Begin("Normal Program");
    bool useModel = flags_ & ENABLE_MODEL;
    if (ImGui::Checkbox("Use Model", &useModel))
    {
        flags_ = useModel ? flags_ | ENABLE_MODEL : flags_ & ~ENABLE_MODEL;
    }
    bool usePlane = flags_ & ENABLE_PLANE;
    if (ImGui::Checkbox("Use Plane", &usePlane))
    {
        flags_ = usePlane ? flags_ | ENABLE_PLANE : flags_ & ~ENABLE_PLANE;
    }
    bool useCube = flags_ & ENABLE_CUBE;
    if (ImGui::Checkbox("Use Cube", &useCube))
    {
        flags_ = useCube ? flags_ | ENABLE_CUBE : flags_ & ~ENABLE_CUBE;
    }
    bool enableSphere = flags_ & ENABLE_SPHERE;
    if (ImGui::Checkbox("Use Sphere", &enableSphere))
    {
        flags_ = enableSphere ? flags_ | ENABLE_SPHERE : flags_ & ~ENABLE_SPHERE;
    }
    bool enableNormal = flags_ & ENABLE_NORMAL_MAP;
    if (ImGui::Checkbox("Enable Normal Map", &enableNormal))
    {
        flags_ = enableNormal ? flags_ | ENABLE_NORMAL_MAP : flags_ & ~ENABLE_NORMAL_MAP;
    }

    ImGui::End();
}
}
