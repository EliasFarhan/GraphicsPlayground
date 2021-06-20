#pragma once

#include "gl/engine.h"
#include "gl/shader.h"
#include "gl/vertex_array.h"
#include "gl/model.h"
#include "gl/camera.h"

namespace gl
{

class HelloCulling : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;
private:
    enum CullingFlags : std::uint8_t
    {
        NONE = 0u,
        CULLING = 1u << 0u,
        BACK_CULLING = 1u << 1u,
        CCW = 1u << 2u
    };
    sdl::Camera3D camera_;
    Model model_;
    ShaderProgram modelShader_;
    Cuboid cube_{glm::vec3(1.0f), glm::vec3()};
    Texture cubeTexture_;
    std::uint8_t flags_ = BACK_CULLING | CCW;

};

}