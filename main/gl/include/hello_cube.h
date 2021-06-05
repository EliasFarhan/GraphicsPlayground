#pragma once

#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"

namespace gl
{
class HelloCube : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    Cuboid cuboid_{glm::vec3(1.0f), glm::vec3(0.0f)};
    ShaderProgram shader_;
    Texture cubeTexture_{};
    constexpr static int cubeNmb = 10;

};
}
