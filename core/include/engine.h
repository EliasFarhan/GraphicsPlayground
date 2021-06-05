#pragma once

#include "SDL_events.h"
#include <chrono>

namespace core
{
using seconds = std::chrono::duration<float, std::ratio<1, 1>>;

class SystemInterface
{
public:
    virtual ~SystemInterface() = default;

    virtual void Init() = 0;

    virtual void Update(seconds dt) = 0;

    virtual void Destroy() = 0;
};

class Program : public SystemInterface
{
public:
    virtual void OnEvent(SDL_Event& event) = 0;

    virtual void DrawImGui() = 0;
};
}
