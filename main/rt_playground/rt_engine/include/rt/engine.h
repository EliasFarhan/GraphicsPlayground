#pragma once
#include <engine.h>

#include "vk_mem_alloc.h"

namespace rt
{

struct Buffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
};
class Engine
{
public:
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

    Engine(core::Program& program);
    void Run();
    std::array<std::uint32_t, 2> GetSize();
    const Swapchain& GetSwapChain() { return swapchain_; }
    VkDevice GetDevice() const { return device_; }

    static Engine& GetInstance();

    bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VmaAllocation& allocation) const;
    void CreateImage(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& imageMemory) const;

    VkCommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;
    std::uint64_t GetBufferDeviceAddress(VkBuffer buffer) const;
protected:
    struct QueueFamilyIndices
    {
        std::optional<std::uint32_t> graphicsFamily;
        std::optional<std::uint32_t> computeFamily;
        std::optional<std::uint32_t> transferFamily;
        std::optional<std::uint32_t> presentFamily;

        [[nodiscard]] bool IsComplete() const
        {
            return graphicsFamily.has_value() && transferFamily.has_value() && computeFamily.has_value() &&
                presentFamily.has_value();
        }
    };




    void Init();
    void Update(core::seconds dt);
    void Destroy();

    bool CreateWindow();
    bool CreateInstance();
    bool SetupDebugMessenger();
    bool CreateSurface();
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateSwapChain();
    bool CreateImageViews();
    bool CreateAllocator();


    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);

    int RateDeviceSuitability(VkPhysicalDevice device) const;
    bool IsDeviceSuitable(VkPhysicalDevice device) const;

    static Engine* engine_;
    core::Program& program_;
    VkCommandPool commandPool_;

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
    VmaAllocator allocator_{};

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
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        // Required by VK_KHR_acceleration_structure
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        // Required for VK_KHR_ray_tracing_pipeline
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        // Required by VK_KHR_spirv_1_4
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
    };
    static constexpr std::array validationLayers = 
    {
        "VK_LAYER_KHRONOS_validation"
    };
};
}
