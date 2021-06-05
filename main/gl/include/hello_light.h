#pragma once


#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"
#include "gl/camera.h"

namespace gl
{
class HelloLight : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:

    gl::sdl::Camera3D camera_;
    gl::Cuboid cube_{glm::vec3(1.0f), glm::vec3(0.0f)};

    gl::ShaderProgram litProgram_;
    gl::ShaderProgram lampProgram_;

    glm::vec3 lampPos_;
    glm::vec3 lightColor_{1,1,1};
    glm::vec3 objectColor_{1.0f,0.23f, 0.1f };
    float ambientStrength_ = 0.1f;
    float diffuseStrength_ = 1.0f;
    float specularStrength_ = 0.5f;
    int specularPow_ = 32;

    static constexpr float lampRadius_ = 10.0f;
    static constexpr float lampPeriod_ = 4.0f;
    float time_ = 0.0f;

};
}