#include <SDL_main.h>

#include "filesystem.h"
#include "hello_texture.h"

namespace gl
{
    void HelloTexture::Init()
    {
        quad_.Init();
        shader_.CreateDefaultProgram(
            "data/shaders/02_hello_texture/texture_quad.vert",
            "data/shaders/02_hello_texture/texture_quad.frag");
        texture_.LoadTexture("data/textures/brickwall.jpg");
        shader_.Bind();
        shader_.SetTexture("ourTexture", texture_, 0);
    }

    void HelloTexture::Update(seconds dt)
    {
        shader_.Bind();
        quad_.Draw();
    }

    void HelloTexture::Destroy()
    {
    }

    void HelloTexture::OnEvent(SDL_Event& event)
    {
    }

    void HelloTexture::DrawImGui()
    {
    }
}