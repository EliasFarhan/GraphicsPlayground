#pragma once
#include <glm/vec3.hpp>

#include "engine.h"
#include "gl/camera.h"
#include "gl/shader.h"
#include "gl/vertex_array.h"


namespace gl
{
class HelloPbrTextured : public core::Program
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
        glm::vec3 position;
        glm::vec3 color;
    };
    std::array<Light, 4> lights_{};
    Sphere sphere_{ 1.0f, glm::vec3() };
    ShaderProgram pbrShader_;
    sdl::Camera3D camera_{};
    Texture albedo_;
    Texture normal_;
    Texture metallic_;
    Texture roughness_;
    Texture ao_;
    bool gammaCorrect_ = true;
};
}
