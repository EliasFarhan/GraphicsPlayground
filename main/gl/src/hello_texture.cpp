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
    ktxTexture_.LoadTexture("data/textures/brickwall.ktx");
}

void HelloTexture::Update(core::seconds dt)
{
    shader_.Bind();
    shader_.SetTexture("ourTexture", usingKtxTexture_?ktxTexture_:texture_, 0);
    quad_.Draw();
}

void HelloTexture::Destroy()
{
    quad_.Destroy();
    shader_.Destroy();
    texture_.Destroy();
    ktxTexture_.Destroy();
}

void HelloTexture::OnEvent(SDL_Event& event)
{
}

void HelloTexture::DrawImGui()
{
    ImGui::Begin("Texture");
    ImGui::Checkbox("KTX", &usingKtxTexture_);
    ImGui::End();

}
}
