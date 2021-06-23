#pragma once

#include "engine.h"
#include <gl/framebuffer.h>
#include <glm/vec3.hpp>
#include <gl/camera.h>
#include <gl/vertex_array.h>
#include <gl/texture.h>
#include <gl/shader.h>

namespace gl
{
class HelloPointShadow : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    struct Transform
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);
        glm::vec3 axis = glm::vec3(0, 1, 0);
        float angle = 0.0f;
    };

    void RenderScene(ShaderProgram& program);

    sdl::Camera3D camera_;
    Cuboid cube_{glm::vec3(1.0f), glm::vec3(0.0f)};

    ShaderProgram lightCubeShader_;
    Camera3D lightCamera_;
    ShaderProgram simpleDepthShader_;

    ShaderProgram cubeShader_;
    Texture cubeTexture_;

    Framebuffer shadowFramebuffer_;
    float dt_ = 0.0f;
    float bias_ = 0.005f;

    std::array<Transform, 5> transforms_ =
            {
                    {
                            {glm::vec3(4.0f, -3.5f, 0.0f), glm::vec3(0.5f)},
                            {glm::vec3(2.0f, 3.0f, 1.0f), glm::vec3(0.75f)},
                            {glm::vec3(-3.0f, -1.0f, 0.0f), glm::vec3(0.5f)},
                            {glm::vec3(-1.5f, 1.0f, 1.5f), glm::vec3(0.5f)},
                            {glm::vec3(-1.5f, 2.0f, -3.0f), glm::vec3(0.75f), glm::normalize(
                                    glm::vec3(1, 0, 1)), glm::radians(60.0f)}
                    }
            };
};
}