#pragma once

#include <chrono>

#include "SDL.h"

#include "glm/vec2.hpp"

namespace gl
{
    using seconds = std::chrono::duration<float, std::ratio<1, 1>>;

    class Program
    {
    public:
        virtual ~Program() = default;
        virtual void Init() = 0;
        virtual void Update(seconds dt) = 0;
        virtual void Destroy() = 0;
        virtual void OnEvent(SDL_Event& event) = 0;
        virtual void DrawImGui() = 0;
    };

    class Engine
    {
    public:
        Engine(Program& program);
        void Run();
    private:
        void Init();
        void Destroy();
        void DrawImGui();

        Program& program_;
        SDL_Window* window_;
        SDL_GLContext glRenderContext_;
        glm::vec2 windowSize_{1024,720};
        float deltaTime_ = 0.0f;
    };
} // namespace gl
