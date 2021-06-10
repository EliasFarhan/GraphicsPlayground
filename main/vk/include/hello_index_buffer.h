#pragma once

#include "vk/engine.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace vk
{
class HelloIndexBuffer : public Program
{
public:
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription GetBindingDescription();
        static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions();
    };
    void Init() override;

    void Update(core::seconds dt) override;

    void Destroy() override;

    void OnEvent(SDL_Event& event) override;

    void DrawImGui() override;

    void CleanupSwapchain() override;

    void RecreateSwapchain() override;

private:

    void CreateGraphicsPipeline();

    void CreateCommands();

    void CreateVertexBuffer();

    void CreateIndexBuffer();

    const std::vector<Vertex> vertices_ = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    const std::vector<std::uint16_t> indices_ = {
            0, 1, 2, 2, 3, 0
    };

    VkPipelineLayout pipelineLayout_{};
    VkPipeline graphicsPipeline_{};
    VkBuffer vertexBuffer_;
    VmaAllocation vertexAllocation_;
    VkBuffer indexBuffer_;
    VmaAllocation indexAllocation_;
};
}
