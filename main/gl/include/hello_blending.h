#pragma once

#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"
#include "gl/camera.h"
#include "gl/model.h"

namespace gl
{
class HelloBlending : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    enum BlendingFlags
    {
        NONE = 0u,
        ENABLE_BLENDING = 1u,
        ENABLE_SORTING = 1u << 1u
    };

    Quad plane_{ glm::vec2(1.0f), glm::vec2() };
    Cuboid cube_{ glm::vec3(1.0f), glm::vec3() };
    ShaderProgram blendingProgram_;
    Texture windowTexture_{};
    Texture cubeTexture_{};
    Texture whiteTexture_{};
    sdl::Camera3D camera_{};
    unsigned int flags_ = NONE;

};


}
