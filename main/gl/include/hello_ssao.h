#pragma once
#include "engine.h"
#include <glm/vec3.hpp>
#include <gl/camera.h>
#include <gl/model.h>
#include <gl/shader.h>
#include <gl/framebuffer.h>
#include <gl/vertex_array.h>

namespace gl
{
class HelloSSAO : public core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:
    struct PointLight
    {
        glm::vec3 position{ 2.0f, 4.0f, -2.0f };
        glm::vec3 color{ 0.2f, 0.2f, 0.7f };

        float constant = 1.0f;
        float linear = 0.09;
        float quadratic = 0.032f;
    };
    void RenderScene(ShaderProgram& shader);
    sdl::Camera3D camera_;
    Texture whiteTexture_;

    ShaderProgram ssaoGeometryShader_;
    ShaderProgram ssaoLightingShader_;
    ShaderProgram ssaoShader_;
    ShaderProgram ssaoBlurShader_;

    Quad screenQuad_{ glm::vec2(2.0f), glm::vec2() };
    Quad plane_{ glm::vec2(1.0f), glm::vec2() };
    Model model_;

    Framebuffer gBuffer_;
    Framebuffer ssaoFramebuffer_;
    Framebuffer ssaoBlurFramebuffer_;
    float ssaoRadius_ = 0.5f;
    float ssaoBias_ = 0.025f;
    unsigned noiseTexture_ = 0;
    const int maxKernelSize_ = 64;
    int kernelSize_ = maxKernelSize_;
    std::vector<glm::vec3> ssaoKernel_;

    PointLight light_;
    bool enableSsao = false;
};
}
