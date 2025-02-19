#include <vk/engine.h>
#include <SDL_vulkan.h>
#include <set>
#include "vk/utility.h"
#include "log.h"

#include "fmt/core.h"

#ifdef TRACY_ENABLE

#include "tracy/Tracy.hpp"

#endif
namespace vk
{
Engine* Engine::instance_ = nullptr;

Engine::Engine(Program& program) : program_(program)
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
#ifdef TRACY_ENABLE
        ZoneNamedN(engineLoop, "Engine Loop", true);
#endif
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
                if (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    RecreateSwapchain();
                }
                program_.OnEvent(event);
            }
        }
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(fenceWait, "Wait For Fence", true);
#endif
            vkWaitForFences(driver_.device, 1,
                            &renderer_.inFlightFences[renderer_.currentFrame],
                            VK_TRUE, UINT64_MAX);
        }
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(acquireImage, "Acquire Image", true);
#endif
            vkAcquireNextImageKHR(driver_.device, swapchain_.swapChain,
                                  UINT64_MAX,
                                  renderer_.imageAvailableSemaphores[renderer_.currentFrame],
                                  VK_NULL_HANDLE,
                                  &renderer_.imageIndex);
        }
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(fenceWait, "Wait Fence If Image Flight", true);
#endif
            if (renderer_.imagesInFlight[renderer_.imageIndex] !=
                VK_NULL_HANDLE)
            {
                vkWaitForFences(driver_.device, 1,
                                &renderer_.imagesInFlight[renderer_.imageIndex],
                                VK_TRUE,
                                UINT64_MAX);
            }
        }
        renderer_.imagesInFlight[renderer_.imageIndex] = renderer_.inFlightFences[renderer_.currentFrame];
        vkResetFences(driver_.device, 1,
                      &renderer_.inFlightFences[renderer_.currentFrame]);

        {
#ifdef TRACY_ENABLE
            ZoneNamedN(presentImage, "Program Update", true);
#endif
            program_.Update(dt);
        }
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(presentImage, "Present Image", true);
#endif
            VkSemaphore signalSemaphores[] = {
                renderer_.renderFinishedSemaphores[renderer_.currentFrame]
            };
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = {swapchain_.swapChain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;
            presentInfo.pImageIndices = &renderer_.imageIndex;
            presentInfo.pResults = nullptr; // Optional
            vkQueuePresentKHR(driver_.presentQueue, &presentInfo);
        }
        renderer_.currentFrame =
            (renderer_.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    Destroy();
}

void Engine::Init()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    //Window Init
    CreateWindow();
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    driver_.physicalDevice = PickPhysicalDevice(driver_.instance,
                                                driver_.surface);
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(driver_.physicalDevice, &properties);
    driver_.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();

    CreateAllocator();

    //Renderer Init
    CreateRenderPass();
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();

    program_.Init();
}

void Engine::Destroy()
{
    vkDeviceWaitIdle(driver_.device);
    program_.Destroy();
    CleanupSwapchain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(driver_.device,
                           renderer_.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(driver_.device,
                           renderer_.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(driver_.device, renderer_.inFlightFences[i], nullptr);
    }
    vmaDestroyAllocator(allocator_);

    vkDestroyDevice(driver_.device, nullptr);
    if (enableDebugMessenger_ && enableValidationLayers_)
    {
        DestroyDebugUtilsMessengerEXT(driver_.instance, debugMessenger_,
                                      nullptr);
    }
    vkDestroySurfaceKHR(driver_.instance, driver_.surface, nullptr);

    vkDestroyInstance(driver_.instance, nullptr);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Engine::CreateInstance()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
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
    appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);

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
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
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
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    if (!enableDebugMessenger_ || !enableValidationLayers_)
    {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(driver_.instance, &createInfo, nullptr,
                                     &debugMessenger_) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to set up debug messenger!");
        std::terminate();
    }
}

void Engine::CreateLogicalDevice()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Creating Logical Device");
    QueueFamilyIndices indices = FindQueueFamilies(driver_.physicalDevice,
                                                   driver_.surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<std::uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
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

    if (vkCreateDevice(driver_.physicalDevice, &createInfo, nullptr,
                       &driver_.device) != VK_SUCCESS)
    {
        core::LogError("Failed to create logical device!");
        std::terminate();
    }
    vkGetDeviceQueue(driver_.device, indices.graphicsFamily.value(), 0,
                     &driver_.graphicsQueue);
    vkGetDeviceQueue(driver_.device, indices.presentFamily.value(), 0,
                     &driver_.presentQueue);
}

void Engine::CreateSurface()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Creating Surface");
    if (!SDL_Vulkan_CreateSurface(window_, driver_.instance, &driver_.surface))
    {
        core::LogError("Failed to create a window surface!");
        std::terminate();
    }
}

void Engine::CreateSwapChain()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Create SwapChain");
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(
        driver_.physicalDevice,
        driver_.surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(
        swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(
        swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);
    core::LogDebug(fmt::format(
        "Swapchain support, minImageCount: {} maxImageCount: {}",
        swapChainSupport.capabilities.minImageCount,
        swapChainSupport.capabilities.maxImageCount));
    swapchain_.minImageCount = swapChainSupport.capabilities.minImageCount;
    swapchain_.imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        swapchain_.imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        swapchain_.imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    core::LogDebug(
        fmt::format("Chosen image count: {}", swapchain_.imageCount));

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = driver_.surface;

    createInfo.minImageCount = swapchain_.imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    QueueFamilyIndices indices = FindQueueFamilies(driver_.physicalDevice,
                                                   driver_.surface);
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
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(driver_.device, &createInfo, nullptr,
                             &swapchain_.swapChain) !=
        VK_SUCCESS)
    {
        core::LogError("[Error] Failed to create swap chain!");
        std::terminate();
    }
    //Adding images
    vkGetSwapchainImagesKHR(driver_.device, swapchain_.swapChain,
                            &swapchain_.imageCount, nullptr);
    swapchain_.images.resize(swapchain_.imageCount);
    vkGetSwapchainImagesKHR(driver_.device, swapchain_.swapChain,
                            &swapchain_.imageCount,
                            swapchain_.images.data());

    swapchain_.imageFormat = surfaceFormat.format;
    swapchain_.extent = extent;
}

void Engine::CreateImageViews()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    swapchain_.imageViews.resize(swapchain_.images.size());
    for (size_t i = 0; i < swapchain_.images.size(); i++)
    {
        swapchain_.imageViews[i] = CreateImageView(driver_.device, swapchain_.images[i], swapchain_.imageFormat,
                                                   VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void Engine::CleanupSwapchain()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    for (auto& framebuffer : renderer_.framebuffers)
    {
        vkDestroyFramebuffer(driver_.device, framebuffer, nullptr);
    }
#ifdef TRACY_ENABLE
    for (std::size_t i = 0; i < tracyContexts_.size(); i++)
    {
        TracyVkDestroy(tracyContexts_[i]);
    }
#endif
    vkFreeCommandBuffers(driver_.device, renderer_.commandPool,
                         static_cast<uint32_t>(renderer_.commandBuffers.size()),
                         renderer_.commandBuffers.data());
    vkDestroyCommandPool(driver_.device, renderer_.commandPool, nullptr);
    vkDestroyRenderPass(driver_.device, renderer_.renderPass, nullptr);

    vkDestroyImageView(driver_.device, renderer_.depthImageView, nullptr);
    vmaDestroyImage(allocator_, renderer_.depthImage, renderer_.depthImageMemory);

    for (auto& imageView : swapchain_.imageViews)
    {
        vkDestroyImageView(driver_.device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(driver_.device, swapchain_.swapChain, nullptr);
}


void Engine::CreateAllocator()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo.physicalDevice = driver_.physicalDevice;
    allocatorInfo.device = driver_.device;
    allocatorInfo.instance = driver_.instance;
    vmaCreateAllocator(&allocatorInfo, &allocator_);
}

VkExtent2D
Engine::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width !=
        std::numeric_limits<std::uint32_t>::max())
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
                                  std::min(capabilities.maxImageExtent.width,
                                           actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height,
                                   std::min(capabilities.maxImageExtent.height,
                                            actualExtent.height));

    return actualExtent;
}

void Engine::CreateRenderPass()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
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

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat(driver_.physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(driver_.device, &renderPassInfo, nullptr,
                           &renderer_.renderPass) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to create render pass!");
        std::terminate();
    }
}

void Engine::CreateSyncObjects()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Create Sync Objects");
    renderer_.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderer_.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderer_.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    renderer_.imagesInFlight.resize(swapchain_.images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(driver_.device, &semaphoreInfo, nullptr,
                              &renderer_.imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateSemaphore(driver_.device, &semaphoreInfo, nullptr,
                              &renderer_.renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
            vkCreateFence(driver_.device, &fenceInfo, nullptr,
                          &renderer_.inFlightFences[i]) != VK_SUCCESS)
        {
            core::LogError("Failed to create sync objects!");
            std::terminate();
        }
    }
}

void Engine::CreateFramebuffers()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Create Framebuffers");
    renderer_.framebuffers.resize(swapchain_.imageViews.size());
    for (size_t i = 0; i < swapchain_.imageViews.size(); i++)
    {

        std::array<VkImageView, 2> attachments = {
            swapchain_.imageViews[i],
            renderer_.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderer_.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
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
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Create Command Pool");
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(
        driver_.physicalDevice,
        driver_.surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional
    if (vkCreateCommandPool(driver_.device, &poolInfo, nullptr,
                            &renderer_.commandPool) !=
        VK_SUCCESS)
    {
        core::LogDebug("Failed to create command pool!");
        std::terminate();
    }
}

void Engine::CreateCommandBuffers()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Create Command Buffers");
    renderer_.commandBuffers.resize(renderer_.framebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer_.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(renderer_.commandBuffers.size());

    if (vkAllocateCommandBuffers(driver_.device, &allocInfo,
                                 renderer_.commandBuffers.data()) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to allocate command buffers!\n");
        std::terminate();
    }

#ifdef TRACY_ENABLE
    tracyContexts_.resize(renderer_.commandBuffers.size());

    for (std::size_t i = 0; i < renderer_.commandBuffers.size(); i++)
    {
        tracyContexts_[i] = TracyVkContext(
            driver_.physicalDevice,
            driver_.device,
            driver_.graphicsQueue,
            renderer_.commandBuffers[i]);
    }
#endif
}

void Engine::CreateDepthResources()
{
    const auto depthFormat = FindDepthFormat(driver_.physicalDevice);
    CreateImage(swapchain_.extent.width, swapchain_.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderer_.depthImage,
                renderer_.depthImageMemory);
    renderer_.depthImageView = CreateImageView(driver_.device, renderer_.depthImage, depthFormat,
                                               VK_IMAGE_ASPECT_DEPTH_BIT);
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

void Engine::RecreateSwapchain()
{
    vkDeviceWaitIdle(driver_.device);

    program_.CleanupSwapchain();
    CleanupSwapchain();

    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    program_.RecreateSwapchain();
}

void
Engine::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkBuffer& buffer,
                     VmaAllocation& allocation)
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
        std::terminate();
    }
}

void
Engine::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, std::size_t size)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    {
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1,
                        &copyRegion);
    }
    EndSingleTimeCommands(commandBuffer);
}

void Engine::CopyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
    EndSingleTimeCommands(commandBuffer);
}

void Engine::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                         VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
                         VmaAllocation& imageMemory) const
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
    allocInfo.commandPool = renderer_.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(driver_.device, &allocInfo, &commandBuffer);

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

    vkQueueSubmit(driver_.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(driver_.graphicsQueue);

    vkFreeCommandBuffers(driver_.device, renderer_.commandPool, 1, &commandBuffer);
}
}
