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
class HelloCutoff : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    Quad plane_{glm::vec2(1.0f), glm::vec2()};
    Cuboid cube_{glm::vec3(1.0f), glm::vec3()};
    ShaderProgram cutoffProgram_;
    Texture grassTexture_{};
    Texture cubeTexture_{};
    Texture whiteTexture_{};
    sdl::Camera3D camera_{};
    bool enableCutoff_ = false;
};
}
