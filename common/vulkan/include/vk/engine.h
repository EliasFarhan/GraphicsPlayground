#pragma once

#include <engine.h>
#include <vector>
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "SDL.h"

#ifdef TRACY_ENABLE

#include "TracyVulkan.hpp"

#endif

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
    float maxAnisotropy;
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
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    std::size_t currentFrame = 0;
    std::uint32_t imageIndex = 0;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkImage depthImage;
    VmaAllocation depthImageMemory;
    VkImageView depthImageView;

};

class VulkanSwapchainRecreationInterface
{
public:
    virtual ~VulkanSwapchainRecreationInterface() = default;
    virtual void CleanupSwapchain() = 0;

    virtual void RecreateSwapchain() = 0;
};

class Program : public core::Program, public VulkanSwapchainRecreationInterface
{

};

class Engine final : public VulkanSwapchainRecreationInterface
{
public:
    Engine(Program& program);

    void Run();

    static Engine& GetInstance()
    { return *instance_; }

    SDL_Window* GetWindow() const;

    Driver& GetDriver();

    Swapchain& GetSwapchain();

    Renderer& GetRenderer();

    VmaAllocator& GetAllocator();

    void RecreateSwapchain() override;

#ifdef TRACY_ENABLE

    std::vector<TracyVkCtx>& GetTracyCtx()
    { return tracyContexts_; }

#endif

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VmaAllocation& allocation);


    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, std::size_t size);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height);

    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& imageMemory) const;

    VkCommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;
    
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
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

    void CleanupSwapchain() override;

    void CreateAllocator();

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void CreateRenderPass();

    void CreateSyncObjects();

    void CreateFramebuffers();

    void CreateCommandPool();

    void CreateCommandBuffers();

    void CreateDepthResources();

    static Engine* instance_;
    SDL_Window* window_ = nullptr;
    Program& program_;
    Driver driver_{};
    Swapchain swapchain_{};
    Renderer renderer_{};
    bool enableValidationLayers_ = true;
    bool enableDebugMessenger_ = true;
    VmaAllocator allocator_;
    VkDebugUtilsMessengerEXT debugMessenger_;
#ifdef TRACY_ENABLE
    std::vector<TracyVkCtx> tracyContexts_;
#endif
};

}
