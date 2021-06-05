#pragma once

#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/vec3.hpp"

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
    std::array<glm::vec3, cubeNmb> positions_{};
    std::array<glm::quat, cubeNmb> quaternions_{};
    glm::vec2 screenSize_;
    float time_ = 0.0f;

};
}
