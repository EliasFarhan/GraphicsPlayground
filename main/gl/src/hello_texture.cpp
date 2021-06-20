#include <hello_texture.h>
#include "imgui.h"

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
    ddsTexture_.LoadTexture("data/textures/brickwall.dds");
}

void HelloTexture::Update([[maybe_unused]] core::seconds dt)
{
    shader_.Bind();
    switch(textureType_)
    {
    case TextureType::NONE:
        shader_.SetTexture("ourTexture", texture_, 0);
        break;
    case TextureType::KTX:
        shader_.SetTexture("ourTexture", ktxTexture_, 0);
        break;
    case TextureType::DDS:
        shader_.SetTexture("ourTexture", ddsTexture_, 0);
        break;
    }
    
    quad_.Draw();
}

void HelloTexture::Destroy()
{
    quad_.Destroy();
    shader_.Destroy();
    texture_.Destroy();
    ktxTexture_.Destroy();
}

void HelloTexture::OnEvent([[maybe_unused]] SDL_Event& event)
{
}

void HelloTexture::DrawImGui()
{
    ImGui::Begin("Texture");
    int currentTextureExtension = static_cast<int>(textureType_);
    ImGui::Combo("Texture Extension", &currentTextureExtension, "None\0KTX\0DDS\0");
    textureType_ = static_cast<TextureType>(currentTextureExtension);
    ImGui::End();

}
}
