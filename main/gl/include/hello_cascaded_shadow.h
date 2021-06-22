#pragma once

#include <gl/framebuffer.h>
#include "engine.h"
#include <gl/vertex_array.h>
#include <gl/texture.h>
#include <gl/camera.h>
#include <gl/model.h>
#include <gl/shader.h>

namespace gl
{
class HelloCascadedShadow : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    enum CascadedShadowFlags : std::uint8_t
    {
        NONE = 0u,
        ENABLE_CASCADE_COLOR = 1u,
        ENABLE_DEPTH_COLOR = 1u << 1u,
        SHOW_DEPTH_TEXTURES = 1u << 2u,
    };

    struct DirectionalLight
    {
        glm::vec3 position = glm::vec3();
        glm::vec3 direction = glm::normalize(glm::vec3(-1.0f));
        glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    };

    [[nodiscard]] Camera2D CalculateOrthoLight(float cascadeNear, float cascadeFar, glm::vec3 lightDir) const;

    void ShadowPass(int cascadeIndex);

    void RenderScene(ShaderProgram& shader);

    sdl::Camera3D camera_;
    Framebuffer shadowFramebuffer_;
    std::array<unsigned int, 3> shadowMaps_{};

    ShaderProgram simpleDepthShader_;
    ShaderProgram shadowShader_;
    ShaderProgram screenShader_;

    static constexpr unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    float cascadedNearRatio_ = 0.1f;
    float cascadedMiddleRatio_ = 0.6f;
    float shadowBias_ = 0.005f;
    std::uint8_t flags_ = ENABLE_DEPTH_COLOR;
    std::array<DirectionalLight, 3> lights_;

    Quad plane_{glm::vec2(1.0f), glm::vec2()};
    Quad screenPlane_{glm::vec2 (2.0f), glm::vec2 ()};
    Model model_;
    Texture brickwall_;
    Texture whiteTexture_;

};
}