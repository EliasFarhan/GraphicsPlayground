#pragma once
#include <engine.h>

#include <volk.h>

#include "vk/utility.h"

namespace rt
{
class Engine
{
public:
    void Run();
protected:

    struct QueueFamilyIndices
    {
        std::optional<std::uint32_t> graphicsFamily;
        std::optional<std::uint32_t> computeFamily;
        std::optional<std::uint32_t> transferFamily;
        std::optional<std::uint32_t> presentFamily;

        [[nodiscard]] bool IsComplete() const
        {
            return graphicsFamily.has_value() && transferFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
        }
    };
    struct Swapchain
    {
        VkSwapchainKHR value;
        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;

        VkFormat imageFormat;
        VkExtent2D extent;

        std::uint32_t minImageCount = 0;
        std::uint32_t imageCount = 0;
    };
    
    void Init();
    void Update(core::seconds dt) ;
    void Destroy();

    bool CreateWindow();
    bool CreateInstance();
    bool SetupDebugMessenger();
    bool CreateSurface();
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateSwapChain();

    int RateDeviceSuitability(VkPhysicalDevice device) const;
    bool IsDeviceSuitable(VkPhysicalDevice device) const;

    QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice device) const;

    SDL_Window* window_ = nullptr;
    VkInstance instance_{};
    VkSurfaceKHR surface_{};
    VkPhysicalDevice physicalDevice_{};
    VkDevice device_{};
    VkQueue graphicsQueue_{};
    VkQueue computeQueue_{};
    VkQueue transferQueue_{};
    VkQueue presentQueue_{};

    //Debug features
    bool enableValidationLayers_ = true;
    bool enableDebugMessenger_ = true;
    bool initResult_ = true;
    VkDebugUtilsMessengerEXT debugMessenger_{};
    Swapchain swapchain_{};


    static constexpr std::array deviceExtensions_ =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        // Ray tracing related extensions required by this sample
        "VK_KHR_acceleration_structure",
        "VK_KHR_ray_tracing_pipeline",
        // Required by VK_KHR_acceleration_structure
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        "VK_KHR_deferred_host_operations",
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        // Required for VK_KHR_ray_tracing_pipeline
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        // Required by VK_KHR_spirv_1_4
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
    };
};
}
