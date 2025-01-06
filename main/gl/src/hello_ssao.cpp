#include <GL/glew.h>
#include "hello_ssao.h"
#include <gl/error.h>
#include <random>
#include <imgui.h>

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
#endif

namespace gl
{
void HelloSSAO::Init()
{

    ssaoBlurShader_.CreateDefaultProgram(
        "data/shaders/22_hello_ssao/ssao.vert",
        "data/shaders/22_hello_ssao/ssao_blur.frag");
    ssaoShader_.CreateDefaultProgram(
        "data/shaders/22_hello_ssao/ssao.vert",
        "data/shaders/22_hello_ssao/ssao.frag");
    ssaoGeometryShader_.CreateDefaultProgram(
        "data/shaders/22_hello_ssao/ssao_geometry.vert",
        "data/shaders/22_hello_ssao/ssao_geometry.frag");
    ssaoLightingShader_.CreateDefaultProgram(
        "data/shaders/22_hello_ssao/ssao.vert",
        "data/shaders/22_hello_ssao/ssao_lighting.frag");
    whiteTexture_.CreateWhiteTexture();
    model_.LoadModel("data/model/nanosuit2/nanosuit.obj");
    screenQuad_.Init();
    plane_.Init();
    camera_.Init();
    // generate sample kernel
    ssaoKernel_.clear();
    ssaoKernel_.reserve(maxKernelSize_);
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis1(-1.0f, 1.0);
    std::uniform_real_distribution<float> dis2(0.0f, 1.0);
    for (int i = 0; i < maxKernelSize_; ++i)
    {
        glm::vec3 sample(
            dis1(gen),
            dis1(gen),
            dis2(gen));
        sample = glm::normalize(sample);
        sample *= dis2(gen);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = std::lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel_.push_back(sample);
    }
    // generate noise texture
    std::array<glm::vec3, 16> ssaoNoise{};
    for (auto& noise : ssaoNoise)
    {
        const auto noiseValue = glm::vec3(dis1(gen), dis1(gen), 0.0f);
        noise = noiseValue;
    }
    glGenTextures(1, &noiseTexture_);
    glBindTexture(GL_TEXTURE_2D, noiseTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    glCheckError();

    //Create framebuffers
    auto& engine = Engine::GetInstance();
    const auto windowSize = engine.GetWindowSize();
    gBuffer_.SetSize({ windowSize[0], windowSize[1] });
    gBuffer_.SetChannelCount(4);
    gBuffer_.SetColorAttachmentNmb(3);
    gBuffer_.SetType(Framebuffer::DEFAULT | Framebuffer::HDR);
    gBuffer_.Create();

    ssaoFramebuffer_.SetSize({ windowSize[0], windowSize[1] });
    ssaoFramebuffer_.SetType(Framebuffer::COLOR_ATTACHMENT_0 | Framebuffer::HDR);
    ssaoFramebuffer_.SetChannelCount(1);
    ssaoFramebuffer_.Create();

    ssaoBlurFramebuffer_.SetSize({ windowSize[0], windowSize[1] });
    ssaoBlurFramebuffer_.SetType(Framebuffer::COLOR_ATTACHMENT_0 | Framebuffer::HDR);
    ssaoBlurFramebuffer_.SetChannelCount(1);
    ssaoBlurFramebuffer_.Create();

    glEnable(GL_DEPTH_TEST);
}

void HelloSSAO::Update(core::seconds dt)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(update, "Update", true);
    TracyGpuNamedZone(updateGpu, "Update", true);
#endif
    camera_.Update(dt);

    const auto view = camera_.GetView();
    const auto projection = camera_.GetProjection();
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(gPass, "Geometry Pass", true);
        TracyGpuNamedZone(gPassGpu, "Geometry Pass", true);
#endif
        // 1. geometry pass: render scene's geometry/color data into gbuffer
        gBuffer_.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ssaoGeometryShader_.Bind();
        ssaoGeometryShader_.SetMat4("view", view);
        ssaoGeometryShader_.SetMat4("projection", projection);
        RenderScene(ssaoGeometryShader_);
    }
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(ssaoPass, "SSAO Pass", true);
        TracyGpuNamedZone(ssaoPassGpu, "SSAO Pass", true);
#endif
        // 2. generate SSAO texture
        ssaoFramebuffer_.Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoShader_.Bind();
        for (unsigned int i = 0; i < 64; i++)
        {
            ssaoShader_.SetVec3("samples[" + std::to_string(i) + "]", ssaoKernel_[i]);
        }
        ssaoShader_.SetMat4("projection", projection);
        ssaoShader_.SetTexture("gPosition", gBuffer_.GetColorTexture(0), 0);
        ssaoShader_.SetTexture("gNormal", gBuffer_.GetColorTexture(1), 1);
        ssaoShader_.SetTexture("texNoise", noiseTexture_, 2);

        ssaoShader_.SetInt("kernelSize", kernelSize_);
        ssaoShader_.SetFloat("radius", ssaoRadius_);
        ssaoShader_.SetFloat("bias", ssaoBias_);
        screenQuad_.Draw();
    }
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(ssaoBlurPass, "Blur Pass", true);
        TracyGpuNamedZone(ssaoBlurPassGpu, "Blur Pass", true);
#endif
        // 3. blur SSAO texture to remove noise

        ssaoBlurFramebuffer_.Bind();
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoBlurShader_.Bind();
        ssaoBlurShader_.SetTexture("ssaoInput", ssaoFramebuffer_.GetColorTexture(0), 0);
        screenQuad_.Draw();
    }
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(lightingPass, "Lighting Pass", true);
        TracyGpuNamedZone(lightingPassGpu, "Lighting Pass", true);
#endif
        // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
        Framebuffer::Unbind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ssaoLightingShader_.Bind();
        const auto lightPosView = glm::vec3(view * glm::vec4(light_.position, 1.0f));
        ssaoLightingShader_.SetVec3("light.position", lightPosView);
        ssaoLightingShader_.SetVec3("light.color", light_.color);
        ssaoLightingShader_.SetFloat("light.linear", light_.linear);
        ssaoLightingShader_.SetFloat("light.quadratic", light_.quadratic);
        ssaoLightingShader_.SetFloat("light.constant", light_.constant);
        ssaoLightingShader_.SetTexture("gPosition", gBuffer_.GetColorTexture(0), 0);
        ssaoLightingShader_.SetTexture("gNormal", gBuffer_.GetColorTexture(1), 1);
        ssaoLightingShader_.SetTexture("gAlbedo", gBuffer_.GetColorTexture(2), 2);
        ssaoLightingShader_.SetTexture("ssao", ssaoBlurFramebuffer_.GetColorTexture(0), 3);
        ssaoLightingShader_.SetInt("enableSSAO", enableSsao);
        screenQuad_.Draw();
    }
}

void HelloSSAO::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    ssaoBlurShader_.Destroy();
    ssaoShader_.Destroy();
    ssaoGeometryShader_.Destroy();
    ssaoLightingShader_.Destroy();

    whiteTexture_.Destroy();
    glDeleteTextures(1, &noiseTexture_);
    model_.Destroy();
    screenQuad_.Destroy();
    plane_.Destroy();

    gBuffer_.Destroy();
    ssaoFramebuffer_.Destroy();
    ssaoBlurFramebuffer_.Destroy();
}

void HelloSSAO::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
    if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        gBuffer_.SetSize({ event.window.data1, event.window.data2 });
        gBuffer_.Reload();
        ssaoBlurFramebuffer_.SetSize({ event.window.data1, event.window.data2 });
        ssaoBlurFramebuffer_.Reload();
        ssaoFramebuffer_.SetSize({ event.window.data1, event.window.data2 });
        ssaoFramebuffer_.Reload();
    }
}

void HelloSSAO::DrawImGui()
{
    ImGui::Begin("SSAO Program");
    ImGui::SliderFloat("Radius", &ssaoRadius_, 0.1f, 0.7f);
    ImGui::SliderFloat("Bias", &ssaoBias_, 0.005f, 0.05f);
    ImGui::SliderInt("Kernel Size", &kernelSize_, 1, maxKernelSize_);
    ImGui::Checkbox("Enable SSAO", &enableSsao);
    ImGui::End();
}

void HelloSSAO::RenderScene(ShaderProgram& shader)
{
    const auto view = camera_.GetView();

    //Draw floor
    auto model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1,0,0));
    model = glm::scale(model, glm::vec3(5.0f));
    //model = glm::Translate(model, glm::vec3::forward * camera_.farPlane / 2.0f);
    shader.SetMat4("model", model);
    shader.SetMat4("normalMatrix", glm::transpose(glm::inverse(view * model)));
    plane_.Draw();
    //Draw model
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0, 1, 0) * 0.1f);
    model = model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1,0,0));
    model = glm::scale(model, glm::vec3( 0.1f));
    shader.SetMat4("model", model);
    shader.SetMat4("normalMatrix", glm::transpose(glm::inverse(view * model)));
    model_.Draw(shader);

    glCheckError();
}
}
