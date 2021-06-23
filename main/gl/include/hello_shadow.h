#pragma once

#include <gl/framebuffer.h>
#include "engine.h"

namespace gl
{

class HelloShadow : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    enum ShadowFlags : std::uint8_t
    {
        NONE = 0u,
        ENABLE_SHADOW = 1u,
        ENABLE_BIAS = 1u << 1u,
        ENABLE_PETER_PANNING = 1u << 2u,
        ENABLE_OVER_SAMPLING = 1u << 3u,
        ENABLE_PCF = 1u << 4u
    };
    struct DirectionalLight
    {
        glm::vec3 lightPos = glm::vec3(10.0f);
        glm::vec3 lightDir = glm::normalize(-glm::vec3(1.0f));
    };

    struct Transform
    {
        glm::vec3 position = glm::vec3(1.0f);
        glm::vec3 scale = glm::vec3(1.0f);
        float angle = 0.0f;
        glm::vec3 axis = glm::vec3(0, 1, 0);
    };

    void RenderScene(ShaderProgram& shader);

    Quad floor_{glm::vec2(5.0f), glm::vec2()};
    Texture floorTexture_;

    Cuboid cube_{glm::vec3(1.0f), glm::vec3()};
    std::array<Transform, 4> cubeTransforms_{
            {
                    {glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.5f)},
                    {glm::vec3(2.0f, 1.5f, 1.0f), glm::vec3(0.5f)},
                    {glm::vec3(4.0f, 0.25f, 4.0f), glm::vec3(0.5f)},
                    {glm::vec3(-1.0f, 1.5f, 2.0f), glm::vec3(
                            0.25f), glm::radians(60.0f), glm::normalize(
                            glm::vec3(1, 0, 1))}}
    };

    Model model_;

    ShaderProgram simpleDepthShader_;
    ShaderProgram modelShader_;

    sdl::Camera3D camera_;
    Camera2D depthCamera_;

    Framebuffer shadowFramebuffer_;
    static constexpr unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    DirectionalLight light_;

    float shadowBias_ = 0.0005f;
    std::uint8_t flags_ = NONE;
};

}