#include <SDL_main.h>

#include "filesystem.h"
#include "gl/engine.h"
#include <GL/glew.h>
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/vertex_array.h"

namespace gl
{
    class HelloCube : public Program
    {
    public:
        void Init() override;

        void Update(seconds dt) override;

        void Destroy() override;

        void OnEvent(SDL_Event &event) override;

        void DrawImGui() override;

    public:

    private:
        Cuboid cuboid_;
        ShaderProgram shader_;
        Texture cubeTexture_;
        constexpr static int cubeNmb = 10;

    };

    void HelloCube::Init()
    {
        cuboid_.Init();
    }

    void HelloCube::Update(seconds dt)
    {

    }

    void HelloCube::Destroy()
    {

    }

    void HelloCube::OnEvent(SDL_Event &event)
    {

    }

    void HelloCube::DrawImGui()
    {

    }
}

int main(int argc, char** argv)
{
    return EXIT_SUCCESS;
}