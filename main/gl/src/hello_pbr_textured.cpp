#include <GL/glew.h>
#include <hello_pbr_textured.h>
#include "fmt/core.h"

namespace gl
{
void HelloPbrTextured::Init()
{
    sphere_.Init();
    pbrShader_.CreateDefaultProgram("data/shaders/24_hello_pbr_textured/pbr.vert",
                                    "data/shaders/24_hello_pbr_textured/pbr.frag");
    camera_.Init();
    camera_.position = glm::vec3(0, 0, 5.0f);
    camera_.LookAt(glm::vec3());
    lights_ = {
    {
    {glm::vec3(-10.0f,  10.0f, 10.0f), glm::vec3(300.0f, 300.0f, 300.0f)},
    {glm::vec3(10.0f,  10.0f, 10.0f),glm::vec3(300.0f, 300.0f, 300.0f)},
    {glm::vec3(-10.0f, -10.0f, 10.0f),glm::vec3(300.0f, 300.0f, 300.0f)},
    {glm::vec3(10.0f, -10.0f, 10.0f),glm::vec3(300.0f, 300.0f, 300.0f)},
    }
    };
    ao_.CreateWhiteTexture();
    albedo_.LoadTexture("data/textures/rustediron2/rustediron2_basecolor.png");
    normal_.LoadTexture("data/textures/rustediron2/rustediron2_normal.png");
    metallic_.LoadTexture("data/textures/rustediron2/rustediron2_metallic.png");
    roughness_.LoadTexture("data/textures/rustediron2/rustediron2_roughness.png");
    glEnable(GL_DEPTH_TEST);
}

void HelloPbrTextured::Update(core::seconds dt)
{
    camera_.Update(dt);
    pbrShader_.Bind();
    pbrShader_.SetTexture("albedoMap", albedo_, 0);
    pbrShader_.SetTexture("normalMap", normal_, 1);
    pbrShader_.SetTexture("metallicMap", metallic_, 2);
    pbrShader_.SetTexture("roughnessMap", roughness_, 3);
    pbrShader_.SetTexture("aoMap", ao_, 4);

    pbrShader_.SetInt("gammaCorrect", gammaCorrect_);

    pbrShader_.SetMat4("view", camera_.GetView());
    pbrShader_.SetVec3("viewPos", camera_.position);
    pbrShader_.SetMat4("projection", camera_.GetProjection());
    for (size_t i = 0; i < lights_.size(); i++)
    {
        pbrShader_.SetVec3(fmt::format("lights[{}].position", i), lights_[i].position);
        pbrShader_.SetVec3(fmt::format("lights[{}].color", i), lights_[i].color);

    }
    pbrShader_.SetMat4("model", glm::mat4(1.0f));
    pbrShader_.SetMat4("normalMatrix", glm::mat4(1.0f));
    sphere_.Draw();
}

void HelloPbrTextured::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    ao_.Destroy();
    albedo_.Destroy();
    normal_.Destroy();
    metallic_.Destroy();
    roughness_.Destroy();
    pbrShader_.Destroy();
    sphere_.Destroy();
}

void HelloPbrTextured::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
    

}

void HelloPbrTextured::DrawImGui()
{
}
}
