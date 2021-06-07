#include <vk/engine.h>
#include <SDL_vulkan.h>
#include <set>
#include "vk/utility.h"
#include "log.h"

#include "fmt/core.h"

#ifdef TRACY_ENABLE

#include "Tracy.hpp"

#endif
namespace vk
{


Engine* Engine::instance_ = nullptr;

Engine::Engine(core::Program& program) : program_(program)
{
    instance_ = this;
}

void Engine::Run()
{
    Init();
    bool isOpen = true;
    std::chrono::time_point<std::chrono::system_clock> clock =
            std::chrono::system_clock::now();
    while (isOpen)
    {
        const auto start = std::chrono::system_clock::now();
        const auto dt = std::chrono::duration_cast<core::seconds>(
                start - clock);
        clock = start;
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(eventManagement, "Event Management", true);
#endif
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    isOpen = false;
                }
                program_.OnEvent(event);
            }
        }
        program_.Update(dt);
    }

    Destroy();
}

void Engine::Init()
{
    //Window Init
    CreateWindow();
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    driver_.physicalDevice = PickPhysicalDevice(driver_.instance, driver_.surface);
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();

    CreateAllocator();

    //Renderer Init
    CreateRenderPass();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSemaphores();

    program_.Init();
}

void Engine::Destroy()
{

    vkDeviceWaitIdle(driver_.device);
    program_.Destroy();
    vmaDestroyAllocator(allocator_);
    for (auto & framebuffer : renderer_.framebuffers)
    {
        vkDestroyFramebuffer(driver_.device, framebuffer, nullptr);
    }
    vkFreeCommandBuffers(driver_.device, renderer_.commandPool,
                         static_cast<uint32_t>(renderer_.commandBuffers.size()),
                         renderer_.commandBuffers.data());

    vkDestroyCommandPool(driver_.device, renderer_.commandPool, nullptr);
    vkDestroySemaphore(driver_.device, renderer_.renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(driver_.device, renderer_.imageAvailableSemaphore, nullptr);
    vkDestroyRenderPass(driver_.device, renderer_.renderPass, nullptr);
    CleanupSwapChain();
    vkDestroyDevice(driver_.device, nullptr);
    if (enableDebugMessenger_ && enableValidationLayers_)
    {
        DestroyDebugUtilsMessengerEXT(driver_.instance, debugMessenger_, nullptr);
    }
    vkDestroySurfaceKHR(driver_.instance, driver_.surface, nullptr);

    vkDestroyInstance(driver_.instance, nullptr);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Engine::CreateInstance()
{
    core::LogDebug("Creating Instance");
    if (enableValidationLayers_ && !CheckValidationLayerSupport())
    {
        core::LogError("Validation layers requested, but not available!");
        std::abort();
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Graphics Playground";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    unsigned int count;
    if (!SDL_Vulkan_GetInstanceExtensions(window_, &count, nullptr))
    {
        core::LogError("[Error] SDL Vulkan, Could not get extensions count");
        std::terminate();
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
        std::terminate();
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

    if (vkCreateInstance(&createInfo, nullptr, &driver_.instance) != VK_SUCCESS)
    {
        core::LogError("Failed to create instance!\n");
        std::terminate();
    }
}

void Engine::CreateWindow()
{
    window_ = SDL_CreateWindow(
            "Vulkan Playground",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1280,
            720,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
}

void Engine::SetupDebugMessenger()
{
    if (!enableDebugMessenger_ || !enableValidationLayers_)
    {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(driver_.instance, &createInfo, nullptr, &debugMessenger_) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to set up debug messenger!");
        std::terminate();
    }
}

void Engine::CreateLogicalDevice()
{
    core::LogDebug("Creating Logical Device");
    QueueFamilyIndices indices = FindQueueFamilies(driver_.physicalDevice, driver_.surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<std::uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                                   indices.presentFamily.value()};
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

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers_)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(driver_.physicalDevice, &createInfo, nullptr, &driver_.device) != VK_SUCCESS)
    {
        core::LogError("Failed to create logical device!");
        std::terminate();
    }
    vkGetDeviceQueue(driver_.device, indices.graphicsFamily.value(), 0, &driver_.graphicsQueue);
    vkGetDeviceQueue(driver_.device, indices.presentFamily.value(), 0, &driver_.presentQueue);
}

void Engine::CreateSurface()
{
    core::LogDebug("Creating Surface");
    if (!SDL_Vulkan_CreateSurface(window_, driver_.instance, &driver_.surface))
    {
        core::LogError("Failed to create a window surface!");
        std::terminate();
    }
}

void Engine::CreateSwapChain()
{
    core::LogDebug("Create SwapChain");
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(driver_.physicalDevice,
                                                                     driver_.surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
    core::LogDebug(fmt::format("Swapchain support, minImageCount: {} maxImageCount: {}",
                               swapChainSupport.capabilities.minImageCount,
                               swapChainSupport.capabilities.maxImageCount));
    swapchain_.minImageCount = swapChainSupport.capabilities.minImageCount;
    swapchain_.imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        swapchain_.imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        swapchain_.imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    core::LogDebug(fmt::format("Image count: {}", swapchain_.imageCount));

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = driver_.surface;

    createInfo.minImageCount = swapchain_.imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    QueueFamilyIndices indices = FindQueueFamilies(driver_.physicalDevice, driver_.surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

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
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(driver_.device, &createInfo, nullptr, &swapchain_.swapChain) !=
        VK_SUCCESS)
    {
        core::LogError("[Error] failed to create swap chain!");
        std::terminate();
    }
    //Adding images
    vkGetSwapchainImagesKHR(driver_.device, swapchain_.swapChain, &swapchain_.imageCount, nullptr);
    swapchain_.images.resize(swapchain_.imageCount);
    vkGetSwapchainImagesKHR(driver_.device, swapchain_.swapChain, &swapchain_.imageCount,
                            swapchain_.images.data());

    swapchain_.imageFormat = surfaceFormat.format;
    swapchain_.extent = extent;
}

void Engine::CreateImageViews()
{
    swapchain_.imageViews.resize(swapchain_.images.size());
    for (size_t i = 0; i < swapchain_.images.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchain_.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchain_.imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(driver_.device, &createInfo, nullptr, &swapchain_.imageViews[i]) !=
            VK_SUCCESS)
        {
            core::LogError("Failed to create image views!");
            std::terminate();
        }
    }
}

void Engine::CleanupSwapChain()
{
    for (auto& imageView : swapchain_.imageViews)
    {
        vkDestroyImageView(driver_.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(driver_.device, swapchain_.swapChain, nullptr);
}


void Engine::CreateAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo.physicalDevice = driver_.physicalDevice;
    allocatorInfo.device = driver_.device;
    allocatorInfo.instance = driver_.instance;
    vmaCreateAllocator(&allocatorInfo, &allocator_);
}

VkExtent2D Engine::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width, height;
    SDL_GetWindowSize(window_, &width, &height);

    VkExtent2D actualExtent = {
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height)
    };

    actualExtent.width = std::max(capabilities.minImageExtent.width,
                                  std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height,
                                   std::min(capabilities.maxImageExtent.height,
                                            actualExtent.height));

    return actualExtent;
}

void Engine::CreateRenderPass()
{
    core::LogDebug("Create Render Pass");
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain_.imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(driver_.device, &renderPassInfo, nullptr, &renderer_.renderPass) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to create render pass!");
        std::terminate();
    }
}

void Engine::CreateSemaphores()
{
    core::LogDebug("Create Semaphore");
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(driver_.device, &semaphoreInfo, nullptr,
                          &renderer_.imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(driver_.device, &semaphoreInfo, nullptr,
                          &renderer_.renderFinishedSemaphore) != VK_SUCCESS)
    {

        core::LogError("Failed to create semaphores!");
        std::terminate();
    }
}

void Engine::CreateFramebuffers()
{
    core::LogDebug("Create Framebuffers");
    renderer_.framebuffers.resize(swapchain_.imageViews.size());
    for (size_t i = 0; i < swapchain_.imageViews.size(); i++)
    {
        VkImageView attachments[] = {
                swapchain_.imageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderer_.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain_.extent.width;
        framebufferInfo.height = swapchain_.extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(driver_.device, &framebufferInfo, nullptr,
                                &renderer_.framebuffers[i]) != VK_SUCCESS)
        {
            core::LogError("Failed to create framebuffer!");
            std::terminate();
        }
    }
}

void Engine::CreateCommandPool()
{
    core::LogDebug("Create Command Pool");
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(driver_.physicalDevice,
                                                              driver_.surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    if (vkCreateCommandPool(driver_.device, &poolInfo, nullptr, &renderer_.commandPool) !=
        VK_SUCCESS)
    {
        core::LogDebug("Failed to create command pool!");
        std::terminate();
    }
}

void Engine::CreateCommandBuffers()
{
    core::LogDebug("Create Command Buffers");
    renderer_.commandBuffers.resize(renderer_.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer_.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(renderer_.commandBuffers.size());

    if (vkAllocateCommandBuffers(driver_.device, &allocInfo, renderer_.commandBuffers.data()) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to allocate command buffers!\n");
        std::terminate();
    }
}

SDL_Window* Engine::GetWindow() const
{
    return window_;
}

Driver& Engine::GetDriver()
{
    return driver_;
}

Swapchain& Engine::GetSwapchain()
{
    return swapchain_;
}

Renderer& Engine::GetRenderer()
{
    return renderer_;
}

VmaAllocator& Engine::GetAllocator()
{
    return allocator_;
}


}
