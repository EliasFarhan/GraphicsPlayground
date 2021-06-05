#pragma once

#include <chrono>
#include <engine.h>

#include "SDL.h"

#include "glm/vec2.hpp"

namespace gl
{
class Engine
{
public:
    Engine(core::Program& program);

    void Run();

    SDL_Window* GetWindow() const
    { return window_; }

    static Engine& GetInstance()
    { return *instance_; }

private:
    void Init();

    void Destroy();

    void DrawImGui();

    core::Program& program_;
    SDL_Window* window_;
    SDL_GLContext glRenderContext_;
    glm::vec2 windowSize_{1024, 720};
    float deltaTime_ = 0.0f;
    static Engine* instance_;
};
} // namespace gl
