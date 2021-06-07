#pragma once

#include <engine.h>
#include <vector>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "SDL.h"

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
    VkRenderPass renderPass;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    std::uint32_t imageIndex = 0;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

};

class Engine
{
public:
    Engine(core::Program& program);


    void Run();

    static Engine& GetInstance()
    { return *instance_; }

    SDL_Window* GetWindow() const;

    Driver& GetDriver();

    Swapchain& GetSwapchain();

    Renderer& GetRenderer();

    VmaAllocator& GetAllocator();


private:

    void Init();

    void Destroy();

    void CreateWindow();

    void CreateInstance();

    void SetupDebugMessenger();

    void CreateLogicalDevice();

    void CreateSurface();

    void CreateSwapChain();

    void CreateImageViews();

    void CleanupSwapChain();

    void CreateAllocator();

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void CreateRenderPass();

    void CreateSemaphores();

    void CreateFramebuffers();

    void CreateCommandPool();

    void CreateCommandBuffers();

    static Engine* instance_;
    SDL_Window* window_ = nullptr;
    core::Program& program_;
    Driver driver_{};
    Swapchain swapchain_{};
    Renderer renderer_{};
    bool enableValidationLayers_ = true;
    bool enableDebugMessenger_ = true;
    VmaAllocator allocator_;
    VkDebugUtilsMessengerEXT debugMessenger_;
};

}
