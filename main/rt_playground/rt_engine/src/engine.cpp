
#include <volk.h>
#include <rt/engine.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <set>

#include "log.h"
#include <fmt/core.h>

namespace rt
{
Engine* Engine::engine_ = nullptr;

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    [[maybe_unused]] void* pUserData)
{
    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        return VK_FALSE;
    const auto msg = pCallbackData->pMessage;
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        core::LogError(msg);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        core::LogWarning(msg);
        break;
    default:
        core::LogDebug(msg);
        break;
    }
    return VK_FALSE;
}

bool Engine::CreateSurface()
{
    core::LogDebug("Creating Surface");
    if (!SDL_Vulkan_CreateSurface(window_, instance_, &surface_))
    {
        core::LogError("Failed to create a window surface!");
        return false;
    }
    return true;
}

bool Engine::PickPhysicalDevice()
{
    core::LogDebug("Picking a physical device");
    std::uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        core::LogError("Failed to find GPUs with Vulkan support!");
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    int maxScore = 0;
    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceInfo;
        vkGetPhysicalDeviceProperties(device, &deviceInfo);
        const int deviceScore = RateDeviceSuitability(device);
        core::LogDebug(fmt::format("Device info: {} with score: {}", deviceInfo.deviceName, deviceScore));

        if (deviceScore > maxScore)
        {
            physicalDevice_ = device;
            maxScore = deviceScore;
        }
    }
    if (physicalDevice_ == VK_NULL_HANDLE)
    {
        core::LogError("Failed to find a suitable GPU!");
        return false;
    }
    return true;
}

bool Engine::CreateLogicalDevice()
{
    core::LogDebug("Creating Logical Device");
    QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<std::uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.transferFamily.value(),
        indices.computeFamily.value(),
        indices.presentFamily.value()
    };
    float queuePriority = 1.0f;
    for (std::uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions_.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions_.data();

    if (enableValidationLayers_)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice_, &createInfo, nullptr,
                       &device_) != VK_SUCCESS)
    {
        core::LogError("Failed to create logical device!");
        return false;
    }
    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0,
                     &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.computeFamily.value(), 0,
                     &computeQueue_);
    vkGetDeviceQueue(device_, indices.transferFamily.value(), 0,
                     &transferQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0,
                     &presentQueue_);
}

bool Engine::CreateSwapChain()
{
    core::LogDebug("Create SwapChain");

    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    //Query SwapChain Support
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &capabilities);

    std::uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, nullptr);

    if (formatCount != 0)
    {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, formats.data());
    }
    //We simply choose double buffering
    const VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    //Choose Swap Surface Format
    VkSurfaceFormatKHR surfaceFormat{};
    for (const auto& availableFormat : formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = availableFormat;
            break;
        }
        surfaceFormat = formats[0];
    }
    //Choose swapchain extend
    VkExtent2D extent;
    if (capabilities.currentExtent.width ==
        std::numeric_limits<std::uint32_t>::max())
    {
        int width, height;
        SDL_GetWindowSize(window_, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width,
                                               actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height,
                                                actualExtent.height));
        extent = actualExtent;
    }
    else
    {
        extent = capabilities.currentExtent;
    }
    core::LogDebug(fmt::format(
        "Swapchain support, minImageCount: {} maxImageCount: {}",
        capabilities.minImageCount,
        capabilities.maxImageCount));
    swapchain_.minImageCount = capabilities.minImageCount;
    swapchain_.imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 &&
        swapchain_.imageCount > capabilities.maxImageCount)
    {
        swapchain_.imageCount = capabilities.maxImageCount;
    }

    core::LogDebug(
        fmt::format("Chosen image count: {}", swapchain_.imageCount));

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;

    createInfo.minImageCount = swapchain_.imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice_);
    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr,
                             &swapchain_.value) !=
        VK_SUCCESS)
    {
        core::LogError("[Error] Failed to create swap chain!");
        return false;
    }
    //Adding images
    vkGetSwapchainImagesKHR(device_, swapchain_.value,
                            &swapchain_.imageCount, nullptr);
    swapchain_.images.resize(swapchain_.imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_.value,
                            &swapchain_.imageCount,
                            swapchain_.images.data());

    swapchain_.imageFormat = surfaceFormat.format;
    swapchain_.extent = extent;
    return true;
}

bool Engine::CreateImageViews()
{
    swapchain_.imageViews.resize(swapchain_.images.size());
    for (std::size_t i = 0; i < swapchain_.images.size(); i++)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchain_.images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchain_.imageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(device_, &viewInfo, nullptr, &swapchain_.imageViews[i]) != VK_SUCCESS)
        {
            core::LogError("Failed to create texture image view!");
            return false;
        }
    }
    return true;
}

bool Engine::CreateAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo.physicalDevice = physicalDevice_;
    allocatorInfo.device = device_;
    allocatorInfo.instance = instance_;
    VmaVulkanFunctions vulkanFunctions;
    vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
    vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
    vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
    vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    vulkanFunctions.vkCreateImage = vkCreateImage;
    vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vulkanFunctions.vkDestroyImage = vkDestroyImage;
    vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vulkanFunctions.vkFreeMemory = vkFreeMemory;
    vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
    vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vulkanFunctions.vkMapMemory = vkMapMemory;
    vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorInfo, &allocator_);
    return true;
}

void Engine::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

int Engine::RateDeviceSuitability(VkPhysicalDevice device) const
{
    int score = 0;
    //Checking queue family and geometry shader
    if (!IsDeviceSuitable(device))
        return 0;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

bool Engine::IsDeviceSuitable(VkPhysicalDevice device) const
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    const auto indices = FindQueueFamilyIndices(device);

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());
    for (auto& extension : availableExtensions)
    {
        core::LogDebug(extension.extensionName);
    }
    std::set<std::string> requiredExtensions(deviceExtensions_.cbegin(), deviceExtensions_.cend());
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }
    const bool extensionsSupported = requiredExtensions.empty();
    if (!extensionsSupported)
    {
        core::LogWarning("Not all extensions supported");
        return false;
    }
    //TODO check adequate swapchain

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        deviceFeatures.geometryShader && indices.IsComplete() && extensionsSupported &&
        deviceFeatures.samplerAnisotropy;
}

Engine::QueueFamilyIndices Engine::FindQueueFamilyIndices(VkPhysicalDevice device) const
{
    // Assign index to queue families that could be found
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (int i = 0; const auto& queueFamily : queueFamilies)
    {
        if (indices.IsComplete())
            break;
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }
        //Checking if available queue family supports graphics command.
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices.computeFamily = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            indices.transferFamily = i;
        }
        i++;
    }
    if (!indices.IsComplete())
    {
        core::LogWarning("Queue family indices not complete");
    }
    return indices;
}

void Engine::Run()
{
    Init();
    if (!initResult_)
        return;
    bool isOpen = true;
    std::chrono::time_point<std::chrono::system_clock> clock =
        std::chrono::system_clock::now();
    while (isOpen)
    {
        const auto start = std::chrono::system_clock::now();
        const auto dt = std::chrono::duration_cast<core::seconds>(
            start - clock);
        clock = start;
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isOpen = false;
            }
        }
        Update(dt);
    }
    Destroy();
}

std::array<std::uint32_t, 2> Engine::GetSize()
{
    return { swapchain_.extent.width, swapchain_.extent.height };
}

Engine& Engine::GetInstance()
{
    return *engine_;
}

bool Engine::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                          VkBuffer& buffer, VmaAllocation& allocation) const
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    allocInfo.requiredFlags = properties;
    if (vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &buffer,
                        &allocation, nullptr) != VK_SUCCESS)
    {
        core::LogError("Failed to create a buffer");
        return false;
    }
    return true;
}

void Engine::CreateImage(std::uint32_t width, std::uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& imageMemory) const
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = properties;

    vmaCreateImage(allocator_, &imageInfo, &allocInfo, &image, &imageMemory, nullptr);
}

VkCommandBuffer Engine::BeginSingleTimeCommands() const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool_;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Engine::EndSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue_);

    vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer);
}

std::uint64_t Engine::GetBufferDeviceAddress(VkBuffer buffer) const
{
    VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;
    return vkGetBufferDeviceAddressEXT(device_, &bufferDeviceAI);
}

Engine::Engine(core::Program& program) : program_(program)
{
    engine_ = this;
}

void Engine::Init()
{
    initResult_ = initResult_ && CreateWindow();
    const auto volkResult = volkInitialize();
    if (volkResult != VK_SUCCESS)
    {
        core::LogError("Could not initialize volk");
        initResult_ = false;
    }
    initResult_ = initResult_ && CreateInstance();
    if (initResult_)
        volkLoadInstance(instance_);
    initResult_ = initResult_ && SetupDebugMessenger();
    initResult_ = initResult_ && CreateSurface();
    initResult_ = initResult_ && PickPhysicalDevice();
    initResult_ = initResult_ && CreateLogicalDevice();
    initResult_ = initResult_ && CreateSwapChain();
    initResult_ = initResult_ && CreateImageViews();
    initResult_ = initResult_ && CreateAllocator();
    if (initResult_)
        program_.Init();
}

void Engine::Update(core::seconds dt)
{
    program_.Update(dt);
}

void Engine::Destroy()
{
    program_.Destroy();
    vkDeviceWaitIdle(device_);
    for (auto& imageView : swapchain_.imageViews)
    {
        vkDestroyImageView(device_, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device_, swapchain_.value, nullptr);

    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(device_, nullptr);
    if (enableDebugMessenger_ && enableValidationLayers_)
    {
        vkDestroyDebugUtilsMessengerEXT(instance_, debugMessenger_,
                                        nullptr);
    }
    vkDestroySurfaceKHR(instance_, surface_, nullptr);

    vkDestroyInstance(instance_, nullptr);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

bool Engine::CreateWindow()
{
    window_ = SDL_CreateWindow(
        "Vulkan Playground",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1280,
        720,
        SDL_WINDOW_VULKAN
    );
    return window_ != nullptr;
}

bool Engine::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Raytracing";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Graphics Playground";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    unsigned int count;
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &count, nullptr))
    {
        core::LogError("[Error] SDL Vulkan, Could not get extensions count");
        return false;
    }

    const char* const additionalExtensions[] =
    {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME, // example additional extension
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME //adding validation layers
    };
    const size_t additionalExtensionsCount =
        sizeof(additionalExtensions) / sizeof(additionalExtensions[0]);
    const size_t extensionCount = count + additionalExtensionsCount;
    std::vector<const char*> extensionNames(extensionCount);

    // get names of required extensions
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &count, &extensionNames[0]))
    {
        core::LogError("SDL Vulkan, Cannot get instance extensions");
        return false;
    }

    // copy additional extensions after required extensions
    for (size_t i = 0; i < additionalExtensionsCount; i++)
    {
        extensionNames[i + count] = additionalExtensions[i];
    }
    SDL_Vulkan_GetInstanceExtensions(window_, &count, &extensionNames[0]);

    core::LogDebug("Vulkan extensions:");
    for (auto& extension : extensionNames)
    {
        core::LogDebug(fmt::format("{}", extension));
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensionNames.size());
    createInfo.ppEnabledExtensionNames = extensionNames.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers_)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS)
    {
        core::LogError("Failed to create instance!\n");
        return false;
    }
    return true;
}

bool Engine::SetupDebugMessenger()
{
    if (!enableDebugMessenger_ || !enableValidationLayers_)
    {
        return true;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessengerCreateInfo(createInfo);
    vkCreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_);
    return true;
}
}
