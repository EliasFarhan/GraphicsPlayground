#pragma once

#include <engine.h>

namespace gl
{
class HelloDeferred : public core::Program
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
        glm::vec3 position;
        glm::vec3 color;
    };

    void RenderScene(gl::ShaderProgram& shader);


    std::array<PointLight, 32> lights_;
    sdl::Camera3D camera_;
    ShaderProgram deferredShader_;
    ShaderProgram lightingShader_;
    ShaderProgram forwardShader_;

    Quad floor_{glm::vec3(10.0f), glm::vec3()};
    Quad screenQuad_{glm::vec3 (2.0f), glm::vec3()};
    Cuboid cube_{glm::vec3(1.0f), glm::vec3(0,0.5f,0)};
    Model model_;

    Texture container_;
    Texture containerSpecular_;
    Texture whiteTexture_;

    Framebuffer gBuffer_;
    bool deferredRendering_ = false;
};
}