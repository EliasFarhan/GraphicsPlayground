#pragma once

#include "vk/engine.h"

namespace vk
{
    class HelloIndexBuffer : public Program
    {
    public:
        void Init() override;
        void Update(core::seconds dt) override;
        void Destroy() override;
        void OnEvent(SDL_Event& event) override;
        void DrawImGui() override;
        void CleanupSwapchain() override;
        void RecreateSwapchain() override;
    };
}
