#pragma once

#include <engine.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace vk
{
    struct Driver
    {
        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkSurfaceKHR surface;
    };


    struct Swapchain
    {
        VkSwapchainKHR swapChain;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;

        VkFormat imageFormat;
        VkExtent2D extent;

        std::uint32_t minImageCount = 0;
        std::uint32_t imageCount = 0;
    };

    struct Renderer
    {
        VkRenderPass renderPass_;
        VkSemaphore imageAvailableSemaphore_;
        VkSemaphore renderFinishedSemaphore_;
        std::uint32_t imageIndex_ = 0;
        std::vector<VkFramebuffer> framebuffers_;
        VkCommandPool commandPool_;
        std::vector<VkCommandBuffer> commandBuffers_;

    };

    class Engine
    {
    public:
        Engine(core::Program& program);
        void Run();
    private:
        void Init();
        void Destroy();

        void CreateInstance();

        core::Program& program_;
        Driver driver_{};
        Swapchain swapchain_{};
    };
    
}
