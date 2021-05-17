#include <SDL_main.h>

#include "engine.h"

namespace gl
{
class HelloTriangle : public Program
{
public:
    void Init() override;
    void Update(seconds dt) override;
    void Destroy() override;
    void OnEvent(SDL_Event& event) override;
    void DrawImGui() override;
};

void HelloTriangle::Init()
{
}

void HelloTriangle::Update(seconds dt)
{
}

void HelloTriangle::Destroy()
{
}

void HelloTriangle::OnEvent(SDL_Event& event)
{
}

void HelloTriangle::DrawImGui()
{
}
}

int main(int argc, char** argv)
{
    gl::HelloTriangle program;
    gl::Engine engine(program);
    engine.Run();
    return EXIT_SUCCESS;
}
