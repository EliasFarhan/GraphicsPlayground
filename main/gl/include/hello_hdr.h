#pragma once
#include "engine.h"
#include "gl/camera.h"
#include "gl/framebuffer.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"

namespace gl
{
class HelloHdr : public core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:
    struct Light
    {
        glm::vec3 lightPos_;
        glm::vec3 lightColor_;
    };
    enum HdrFlags : std::uint8_t
    {
        NONE = 0u,
        ENABLE_HDR = 1u << 0u,
        ENABLE_REINHARD = 1u << 1u
    };

    static constexpr std::array<Light, 4> lights_{ {
        { glm::vec3(0.0f, 0.0f, 27.5f), glm::vec3(200.0f, 200.0f, 200.0f)},
        { glm::vec3(-1.4f, -1.9f, 9.0f),glm::vec3(0.1f, 0.0f, 0.0f) },
        { glm::vec3(0.0f, -1.8f, 4.0f), glm::vec3(0.0f, 0.0f, 0.2f) },
        { glm::vec3(0.8f, -1.7f, 6.0f), glm::vec3(0.0f, 0.1f, 0.0f) }
        }};

    Framebuffer hdrFrambuffer_;
    Quad hdrQuad_{glm::vec2(2.0f), glm::vec2()};
    ShaderProgram hdrShader_;

    Cuboid cube_{glm::vec3(5, 5, 50), glm::vec3(0, 0, 10)};
    Texture cubeTexture_;
    ShaderProgram cubeShader_;

    sdl::Camera3D camera_;
    float exposure_ = 1.0f;
    std::uint8_t flags_ = ENABLE_REINHARD;
};
}
