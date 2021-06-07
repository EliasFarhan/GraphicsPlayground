#pragma once

#include <engine.h>
#include "vulkan/vulkan.h"

namespace vk
{
class HelloTriangle : public core::Program
{
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

private:
    void CreateGraphicsPipeline();
    void CreateCommands();

    VkPipelineLayout pipelineLayout_;
    VkPipeline graphicsPipeline_;
};
}