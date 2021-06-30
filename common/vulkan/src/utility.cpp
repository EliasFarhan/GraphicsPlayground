#include <iostream>
#include <set>
#include <vector>
#include <vk/utility.h>
#include <log.h>
#include <fmt/format.h>

namespace vk
{
bool CheckValidationLayerSupport()
{
    std::uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    core::LogDebug("Available layers: ");
    for (auto& layerProperties : availableLayers)
    {
        core::LogDebug(layerProperties.layerName);
    }

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

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

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                          "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                           "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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

bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    QueueFamilyIndices indices = FindQueueFamilies(device, surface);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);

        swapChainAdequate =
            !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        deviceFeatures.geometryShader && indices.IsComplete() && extensionsSupported &&
        swapChainAdequate && deviceFeatures.samplerAnisotropy;
}

int RateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    int score = 0;
    //Checking queue family and geometry shader
    if (!IsDeviceSuitable(device, surface))
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

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    // Assign index to queue families that could be found
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (indices.IsComplete())
            break;
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport)
        {
            indices.presentFamily = i;
        }
        if (queueFamily.queueFlags &
            VK_QUEUE_GRAPHICS_BIT) //Checking if available queue family supports graphics command.
        {
            indices.graphicsFamily = i;
        }
        i++;
    }
    return indices;
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());
    for (auto& extension : availableExtensions)
    {
        core::LogDebug(extension.extensionName);
    }
    std::set<std::string> requiredExtensions(deviceExtensions.cbegin(), deviceExtensions.cend());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    std::uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    std::uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                  details.presentModes.data());
    }
    return details;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPhysicalDevice PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    core::LogDebug("Picking a physical device");
    VkPhysicalDevice physicalDevice;
    std::uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        core::LogError("Failed to find GPUs with Vulkan support!");
        std::terminate();
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    int maxScore = 0;
    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceInfo;
        vkGetPhysicalDeviceProperties(device, &deviceInfo);
        core::LogDebug(fmt::format("Device info: {}", deviceInfo.deviceName));
        const int deviceScore = RateDeviceSuitability(device, surface);
        if (deviceScore > maxScore)
        {
            physicalDevice = device;
            maxScore = deviceScore;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        core::LogError("Failed to find a suitable GPU!");
        std::terminate();
    }
    return physicalDevice;
}

VkPresentModeKHR ChooseSwapPresentMode([[maybe_unused]] const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    //Choosing Double Buffering
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkShaderModule CreateShaderModule(const core::BufferFile& bufferFile, VkDevice device)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = bufferFile.dataLength;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(bufferFile.dataBuffer);
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        core::LogError("Failed to create shader module!");
        std::terminate();
    }
    return shaderModule;
}
}
