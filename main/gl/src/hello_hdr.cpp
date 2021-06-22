#include <GL/glew.h>
#include "hello_hdr.h"
#include <imgui.h>


namespace gl
{
void HelloHdr::Init()
{
    cube_.Init();
    cubeShader_.CreateDefaultProgram("data/shaders/19_hello_hdr/tunnel.vert", "data/shaders/19_hello_hdr/tunnel.frag");
    cubeTexture_.LoadTexture("data/textures/brickwall.jpg",
                             Texture::MIRROR_REPEAT_WRAP | Texture::GAMMA_CORRECTION | Texture::MIPMAP |
                             Texture::SMOOTH);
    hdrQuad_.Init();
    hdrFrambuffer_.SetType(Framebuffer::HDR | Framebuffer::COLOR_ATTACHMENT_0 | Framebuffer::DEPTH_RBO);
    const auto windowSize = Engine::GetInstance().GetWindowSize();
    hdrFrambuffer_.SetSize({windowSize[0], windowSize[1]});
    hdrFrambuffer_.Create();

    hdrShader_.CreateDefaultProgram("data/shaders/19_hello_hdr/hdr_screen.vert",
                                    "data/shaders/19_hello_hdr/hdr_screen.frag");
    camera_.Init();
    camera_.position = glm::vec3();
    camera_.LookAt(glm::vec3(0, 0, 1));

    glEnable(GL_DEPTH_TEST);
}

void HelloHdr::Update(core::seconds dt)
{
    camera_.Update(dt);

    hdrFrambuffer_.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cubeShader_.Bind();
    cubeShader_.SetMat4("view", camera_.GetView());
    cubeShader_.SetMat4("projection", camera_.GetProjection());
    cubeShader_.SetMat4("model", glm::mat4(1.0f));
    cubeShader_.SetMat4("transposeInverseModel", glm::mat4(1.0f));
    cubeShader_.SetTexture("diffuseTexture", cubeTexture_, 0);
    for (size_t i = 0; i < lights_.size(); i++)
    {
        cubeShader_.SetVec3("lights[" + std::to_string(i) + "].Position", lights_[i].lightPos_);
        cubeShader_.SetVec3("lights[" + std::to_string(i) + "].Color", lights_[i].lightColor_);
    }
    cubeShader_.SetInt("lightNmb", lights_.size());
    cubeShader_.SetInt("inverseNormals", true);
    cube_.Draw();
    hdrFrambuffer_.Unbind();
    hdrShader_.Bind();
    hdrShader_.SetTexture("hdrBuffer", hdrFrambuffer_.GetColorTexture(), 0);
    hdrShader_.SetInt("hdr", flags_ & ENABLE_HDR);
    hdrShader_.SetInt("reinhard", flags_ & ENABLE_REINHARD);
    hdrShader_.SetFloat("exposure", exposure_);
    hdrQuad_.Draw();

}

void HelloHdr::Destroy()
{
    glDisable(GL_DEPTH_TEST);
    cube_.Destroy();
    cubeShader_.Destroy();
    cubeTexture_.Destroy();

    hdrQuad_.Destroy();
    hdrFrambuffer_.Destroy();
    hdrShader_.Destroy();

}

void HelloHdr::OnEvent(SDL_Event& event)
{
    camera_.OnEvent(event);
    if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
    {
        hdrFrambuffer_.SetSize({ event.window.data1, event.window.data2 });
        hdrFrambuffer_.Reload();
    }
}

void HelloHdr::DrawImGui()
{
    ImGui::Begin("Hdr Program");
    bool enableHdr = flags_ & ENABLE_HDR;
    if (ImGui::Checkbox("Enable Hdr", &enableHdr))
    {
        flags_ = enableHdr ? flags_ | ENABLE_HDR : flags_ & ~ENABLE_HDR;
    }
    if (enableHdr)
    {
        bool enableReinhard = flags_ & ENABLE_REINHARD;
        if (ImGui::Checkbox("Enable Reinhard Tonemapping", &enableReinhard))
        {
            flags_ = enableReinhard ? flags_ | ENABLE_REINHARD : flags_ & ~ENABLE_REINHARD;
        }
        if (!(flags_ & ENABLE_REINHARD))
        {
            ImGui::SliderFloat("Exposure", &exposure_, 0.1f, 6.0f);
        }
    }
    ImGui::End();
}
}
