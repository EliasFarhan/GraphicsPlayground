#pragma once
#include <array>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "vk/engine.h"

namespace vk
{

class HelloTexture : public Program
{
public:
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription GetBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 3>
        GetAttributeDescriptions();
    };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
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

    void CreateDescriptorSetLayout();

    void CreateUniformBuffers();

    void CreateDescriptorPool();

    void CreateDescriptorSets();

    void UpdateUniformBuffers(core::seconds duration);

    void CreateTextureImage();

    void CreateTextureImageView();
    void CreateTextureSampler();

    static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    const std::vector<Vertex> vertices_ = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    };
    const std::vector<std::uint16_t> indices_ = {
            0, 1, 2, 2, 3, 0
    };
    VkDescriptorPool descriptorPool_{};
    VkDescriptorSetLayout descriptorSetLayout_{};
    std::vector<VkDescriptorSet> descriptorSets_;
    VkPipelineLayout pipelineLayout_{};
    VkPipeline graphicsPipeline_{};
    VkBuffer vertexBuffer_;
    VmaAllocation vertexAllocation_;
    VkBuffer indexBuffer_;
    VmaAllocation indexAllocation_;
    VkImage textureImage_;
    VmaAllocation textureImageMemory_;
    VkImageView textureImageView_;
    VkSampler textureSampler_;

    std::vector<VkBuffer> uniformBuffers_;
    std::vector<VmaAllocation> uniformBuffersAllocations_;

    float dt_ = 0.0f;
};

}
