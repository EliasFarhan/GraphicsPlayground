#include <SDL_main.h>

#include "filesystem.h"
#include "gl/engine.h"
#include "GL/glew.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"

namespace gl
{
    class HelloTexture : public Program
    {
    public:
        void Init() override;
        void Update(seconds dt) override;
        void Destroy() override;
        void OnEvent(SDL_Event& event) override;
        void DrawImGui() override;
    private:
        Quad quad_{glm::vec2(1.0f), glm::vec2(0.0f)};
        ShaderProgram shader_;
        Texture texture_{};
    };

    void HelloTexture::Init()
    {
        quad_.Init();
        shader_.CreateDefaultProgram(
            "data/shaders/texture_quad.vert", 
            "data/shaders/texture_quad.frag");
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
int main(int argc, char** argv)
{
    core::Filesystem filesystem;
    gl::HelloTexture program;
    gl::Engine engine(program);
    engine.Run();
    return EXIT_SUCCESS;
}