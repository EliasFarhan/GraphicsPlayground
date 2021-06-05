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
class HelloModel : public core::Program
{
public:
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    Model model_;
    sdl::Camera3D camera_;

    ShaderProgram shader_;
};
}