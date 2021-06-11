#pragma once

#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"


namespace gl
{
class HelloTexture : public core::Program
{
public:
    void Init() override;
    void Update(core::seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
private:
    Quad quad_{glm::vec2(1.0f), glm::vec2(0.0f)};
    ShaderProgram shader_;
    Texture texture_{};
    Texture ktxTexture_{};
    bool usingKtxTexture_ = false;
};
}