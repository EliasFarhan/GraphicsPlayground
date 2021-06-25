#include "hello_deferred.h"
#include <random>
#include "fmt/core.h"

#ifdef TRACY_ENABLE

#include "Tracy.hpp"
#include "TracyOpenGL.hpp"

#endif

namespace gl
{

void HelloDeferred::Init()
{
    floor_.Init();
    screenQuad_.Init();
    cube_.Init();

    deferredShader_.CreateDefaultProgram("data/shaders/21_hello_deferred/deferred.vert",
                                         "data/shaders/21_hello_deferred/deferred.frag");
    lightingShader_.CreateDefaultProgram("data/shaders/21_hello_deferred/lighting.vert",
                                         "data/shaders/21_hello_deferred/lighting.frag");
    forwardShader_.CreateDefaultProgram("data/shaders/21_hello_deferred/forward.vert",
                                        "data/shaders/21_hello_deferred/forward.frag");
    whiteTexture_.CreateWhiteTexture();
    container_.LoadTexture("data/textures/container2.png");
    containerSpecular_.LoadTexture("data/textures/container2_specular.png");

    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");

    camera_.Init();
    camera_.position = glm::vec3(0, 3, -3);
    camera_.LookAt(glm::vec3());

    const auto windowSize = Engine::GetInstance().GetWindowSize();
    gBuffer_.SetSize({windowSize[0], windowSize[1]});
    gBuffer_.SetColorAttachmentNmb(3);
    gBuffer_.SetType(Framebuffer::HDR | Framebuffer::COLOR_ATTACHMENT_0 |
                     Framebuffer::DEPTH_RBO);
    gBuffer_.Create();

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.0f, 1.0f);
    for (int x = -2; x < 3; x++)
    {
        for (int z = -2; z < 3; z++)
        {
            lights_[(x + 2) + (z + 2) * 5].color = glm::vec3(dis(gen), dis(gen),
                                                             dis(gen));
            lights_[(x + 2) + (z + 2) * 5].position = glm::vec3(2.5f * float(x),
                                                                1.5f, 2.5f *
                                                                      (float(z) +
                                                                       2.0f));
        }
    }
    glCheckError();
    glEnable(GL_DEPTH_TEST);
}

void HelloDeferred::Update(core::seconds dt)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(update, "Update", true);
    TracyGpuNamedZone(updateGpu, "Update", true);
#endif
    camera_.Update(dt);

    if (deferredRendering_)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(deferred, "Deferred Rendering", true);
        TracyGpuNamedZone(deferredGpu, "Deferred Rendering", true);
#endif
        gBuffer_.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        deferredShader_.Bind();
        deferredShader_.SetMat4("view", camera_.GetView());
        deferredShader_.SetMat4("projection", camera_.GetProjection());

        RenderScene(deferredShader_);

        Framebuffer::Unbind();
#ifdef TRACY_ENABLE
        ZoneNamedN(lighting, "Lighting Pass", true);
        TracyGpuNamedZone(lightingGpu, "Lighting Pass", true);
#endif
        lightingShader_.Bind();
        for (int i = 0; i < 32; i++)
        {
            lightingShader_.SetVec3(
                    "lights[" + std::to_string(i) + "].position",
                    lights_[i].position);
            lightingShader_.SetVec3("lights[" + std::to_string(i) + "].color",
                                    lights_[i].color);
        }
        lightingShader_.SetTexture("gPosition", gBuffer_.GetColorTexture(0), 0);
        lightingShader_.SetTexture("gNormal", gBuffer_.GetColorTexture(1), 1);
        lightingShader_.SetTexture("gAlbedoSpec", gBuffer_.GetColorTexture(2),
                                   2);
        lightingShader_.SetVec3("viewPos", camera_.position);
        screenQuad_.Draw();
    }
    else
    {

#ifdef TRACY_ENABLE
        ZoneNamedN(forward, "Forward Rendering", true);
        TracyGpuNamedZone(forwardGpu, "Forward Rendering", true);
#endif
        forwardShader_.Bind();
        forwardShader_.SetMat4("view", camera_.GetView());
        forwardShader_.SetMat4("projection", camera_.GetProjection());
        forwardShader_.SetVec3("viewPos", camera_.position);
        for (int i = 0; i < 32; i++)
        {
            forwardShader_.SetVec3(fmt::format("lights[{}].position", i),
                                   lights_[i].position);
            forwardShader_.SetVec3(fmt::format("lights[{}].color", i),
                                   lights_[i].color);
        }
        RenderScene(forwardShader_);
    }
}

void HelloDeferred::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    cube_.Destroy();
    screenQuad_.Destroy();
    floor_.Destroy();
    forwardShader_.Destroy();
    lightingShader_.Destroy();
    deferredShader_.Destroy();
    gBuffer_.Destroy();
    whiteTexture_.Destroy();
    containerSpecular_.Destroy();
    container_.Destroy();

}

void HelloDeferred::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        gBuffer_.SetSize({event.window.data1, event.window.data2});
        gBuffer_.Reload();
    }
}

void HelloDeferred::DrawImGui()
{
    ImGui::Begin("Deferred Program");
    ImGui::Checkbox("Enable Deferred", &deferredRendering_);
    ImGui::End();
}

void HelloDeferred::RenderScene(ShaderProgram& shader)
{
    for (int x = -2; x < 3; x++)
    {
        for (int z = -2; z < 3; z++)
        {

            auto model = glm::mat4(1.0f);
            {
#ifdef TRACY_ENABLE
                ZoneNamedN(drawModel, "Draw Model", true);
                TracyGpuNamedZone(drawModelGpu, "Draw Model", true);
#endif
                shader.SetTexture("texture_diffuse1", container_, 0);
                shader.SetTexture("texture_specular1", containerSpecular_,
                                  1);
                model = glm::translate(model, glm::vec3(2.0f * float(x), 0.0f,
                                                        2.0f *
                                                        (float(z) + 2.0f)));
                shader.SetMat4("model", model);
                shader.SetMat4("transposeInverseModel",
                               glm::transpose(glm::inverse(model)));
                cube_.Draw();
            }
            {
#ifdef TRACY_ENABLE
                ZoneNamedN(drawModel, "Draw Cube", true);
                TracyGpuNamedZone(drawModelGpu, "Draw Cube", true);
#endif
                model = glm::mat4(1.0f);
                model = glm::translate(model,
                                       glm::vec3(2.0f * (float(x) + 0.5f), 0.0f,
                                                 2.0f * (float(z) + 2.5f)));
                model = glm::scale(model, glm::vec3(0.2f));

                shader.SetMat4("model", model);
                shader.SetMat4("transposeInverseModel",
                               glm::transpose(glm::inverse(model)));
                model_.Draw(shader);
            }
        }
    }
#ifdef TRACY_ENABLE
    ZoneNamedN(drawModel, "Draw Floor", true);
    TracyGpuNamedZone(drawModelGpu, "Draw Floor", true);
#endif
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0,0,1) * camera_.farPlane / 2.0f);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1,0,0));
    model = glm::scale(model, glm::vec3(camera_.farPlane));
    shader.SetTexture("texture_diffuse1", whiteTexture_, 0);
    shader.SetTexture("texture_specular1", whiteTexture_, 1);
    shader.SetMat4("model", model);
    shader.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));
    floor_.Draw();
}
}