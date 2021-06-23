#include <GL/glew.h>
#include "hello_bloom.h"
#include "imgui.h"

namespace gl
{
void HelloBloom::Init()
{
    cube_.Init();
    cubeTexture_.LoadTexture("data/textures/container.jpg");

    cubeShader_.CreateDefaultProgram("data/shaders/20_hello_bloom/cube.vert", "data/shaders/20_hello_bloom/cube.frag");
    lightShader_.CreateDefaultProgram("data/shaders/20_hello_bloom/cube.vert",
                                      "data/shaders/20_hello_bloom/light_cube.frag");
    blurShader_.CreateDefaultProgram("data/shaders/20_hello_bloom/blur.vert", "data/shaders/20_hello_bloom/blur.frag");
    bloomShader_.CreateDefaultProgram("data/shaders/20_hello_bloom/bloom.vert",
                                      "data/shaders/20_hello_bloom/bloom.frag");

    screenPlane_.Init();
    camera_.Init();
    camera_.position = glm::vec3(0, 0, 5);
    camera_.LookAt(glm::vec3());

    const auto windowSize = Engine::GetInstance().GetWindowSize();
    hdrFramebuffer_.SetSize({windowSize[0], windowSize[1]});
    hdrFramebuffer_.SetType(Framebuffer::HDR | Framebuffer::DEFAULT);
    hdrFramebuffer_.SetColorAttachmentNmb(2);
    hdrFramebuffer_.Create();

    for (auto& pingpongFramebuffer : pingpongFramebuffers_)
    {
        pingpongFramebuffer.SetSize({windowSize[0], windowSize[1]});
        pingpongFramebuffer.SetType(Framebuffer::HDR | Framebuffer::DEFAULT);
        pingpongFramebuffer.Create();
    }
    glEnable(GL_DEPTH_TEST);
}

void HelloBloom::Update(core::seconds dt)
{
    camera_.Update(dt);
    const auto view = camera_.GetView();
    const auto projection = camera_.GetProjection();
    hdrFramebuffer_.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cubeShader_.Bind();
    cubeShader_.SetMat4("view", view);
    cubeShader_.SetMat4("projection", projection);
    cubeShader_.SetTexture("diffuseTexture", cubeTexture_, 0);

    for (size_t i = 0; i < lights_.size(); i++)
    {
        cubeShader_.SetVec3("lights[" + std::to_string(i) + "}.Position", lights_[i].position_);
        cubeShader_.SetVec3("lights[" + std::to_string(i) + "].Color", lights_[i].color_);
    }
    cubeShader_.SetInt("lightNmb", lights_.size());
    cubeShader_.SetVec3("viewPos", camera_.position);
    for (const auto& transform : cubeTransforms_)
    {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, transform.position);
        model = glm::rotate(model, transform.angle, transform.axis);
        model = glm::scale(model, transform.scale);
        cubeShader_.SetMat4("model", model);
        cubeShader_.SetMat4("transposeInverseModel", glm::transpose(glm::inverse(model)));

        cube_.Draw();
    }
    lightShader_.Bind();
    lightShader_.SetMat4("view", view);
    lightShader_.SetMat4("projection", projection);
    for (const auto& light : lights_)
    {
        auto model = glm::mat4(1.0f);
        model = glm::translate(model, light.position_);
        model = glm::scale(model, glm::vec3(0.25f));
        lightShader_.SetMat4("model", model);
        lightShader_.SetVec3("lightColor", light.color_);
        cube_.Draw();
    }

    bool horizontal = true;
    if (enableBloom_)
    {
        bool firstIteration = true;
        blurShader_.Bind();
        blurShader_.SetInt("image", 0);
        for (int i = 0; i < blurAmount_; i++)
        {
            pingpongFramebuffers_[horizontal].Bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            blurShader_.SetInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, firstIteration
                                             ? hdrFramebuffer_.GetColorTexture(1)
                                             : pingpongFramebuffers_[!horizontal].GetColorTexture());
            // bind texture of other framebuffer (or scene if first iteration)
            screenPlane_.Draw();
            horizontal = !horizontal;
            if (firstIteration)
                firstIteration = false;
        }
    }
    Framebuffer::Unbind();
    // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
    // --------------------------------------------------------------------------------------------------------------------------
    bloomShader_.Bind();
    bloomShader_.SetTexture("scene", hdrFramebuffer_.GetColorTexture(), 0);
    bloomShader_.SetTexture("bloomBlur", pingpongFramebuffers_[!horizontal].GetColorTexture(), 1);
    bloomShader_.SetInt("bloom", enableBloom_);
    bloomShader_.SetFloat("exposure", exposure_);
    screenPlane_.Draw();
}

void HelloBloom::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    cube_.Destroy();
    cubeTexture_.Destroy();
    screenPlane_.Destroy();

    bloomShader_.Destroy();
    blurShader_.Destroy();
    cubeShader_.Destroy();
    lightShader_.Destroy();

    hdrFramebuffer_.Destroy();
    std::ranges::for_each(pingpongFramebuffers_, [](auto& f) { f.Destroy(); });
}

void HelloBloom::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        hdrFramebuffer_.SetSize({event.window.data1, event.window.data2});
        hdrFramebuffer_.Reload();
        for (auto& ppF : pingpongFramebuffers_)
        {
            ppF.SetSize({event.window.data1, event.window.data2});
            ppF.Reload();
        }
    }
}

void HelloBloom::DrawImGui()
{
    ImGui::Begin("Bloom Program");
    ImGui::Checkbox("Enable Bloom", &enableBloom_);

    ImGui::SliderFloat("Exposure", &exposure_, 0.1f, 10.0f);
    ImGui::SliderInt("Blur Amount", &blurAmount_, 2, 20);
    ImGui::End();
}
}
