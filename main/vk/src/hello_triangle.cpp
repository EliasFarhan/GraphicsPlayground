#include <log.h>
#include <exception>
#include <filesystem.h>
#include <vk/engine.h>
#include <vk/utility.h>
#include "hello_triangle.h"

namespace vk
{

void HelloTriangle::Init()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    CreateGraphicsPipeline();
    CreateCommands();
}

void HelloTriangle::Update(core::seconds dt)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(triangleLoop, "Triangle Loop", true);
#endif
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();
    auto& renderer = engine.GetRenderer();


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {renderer.imageAvailableSemaphores[renderer.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer.commandBuffers[renderer.imageIndex];

    VkSemaphore signalSemaphores[] = {renderer.renderFinishedSemaphores[renderer.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;


    if (vkQueueSubmit(driver.graphicsQueue, 1, &submitInfo, renderer.inFlightFences[renderer.currentFrame]) != VK_SUCCESS)
    {
        core::LogError("Failed to submit draw command buffer!");
        std::terminate();
    }


}

void HelloTriangle::Destroy()
{
    CleanupSwapchain();
}

void HelloTriangle::OnEvent(SDL_Event& event)
{

}

void HelloTriangle::DrawImGui()
{

}

void HelloTriangle::CreateGraphicsPipeline()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Create Graphics Pipeline");
    const auto& filesystem = core::FilesystemLocator::get();
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();

    core::BufferFile vertexShaderFile = filesystem.LoadFile(
            "data/shaders/01_hello_triangle/triangle.vert.spv");
    core::BufferFile fragmentShaderFile = filesystem.LoadFile(
            "data/shaders/01_hello_triangle/triangle.frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertexShaderFile, driver.device);
    VkShaderModule fragShaderModule = CreateShaderModule(fragmentShaderFile, driver.device);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    //We don't give any input buffer
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.extent.width);
    viewport.height = static_cast<float>(swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain.extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(driver.device, &pipelineLayoutInfo, nullptr, &pipelineLayout_) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to create pipeline layout!\n");
        std::terminate();
    }
    auto renderPass = engine.GetRenderer().renderPass;
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional


    if (vkCreateGraphicsPipelines(driver.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &graphicsPipeline_) != VK_SUCCESS)
    {
        core::LogError("Failed to create graphics pipeline!\n");
        std::terminate();
    }
    vkDestroyShaderModule(driver.device, vertShaderModule, nullptr);
    vkDestroyShaderModule(driver.device, fragShaderModule, nullptr);
}

void HelloTriangle::CreateCommands()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    core::LogDebug("Create Commands");
    auto& engine = Engine::GetInstance();
    auto& renderer = engine.GetRenderer();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();

    auto& commandBuffers = renderer.commandBuffers;
    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
#ifdef TRACY_ENABLE
        auto& tracyCtx = engine.GetTracyCtx();
#endif
        {
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                core::LogError("Failed to begin recording command buffer!");
                std::terminate();
            }
#ifdef TRACY_ENABLE
            TracyVkZone(tracyCtx[i], renderer.commandBuffers[i], "Hello Triangle");
#endif
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderer.renderPass;
            renderPassInfo.framebuffer = renderer.framebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapchain.extent;

            VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);
        }
#ifdef TRACY_ENABLE
        TracyVkCollect(tracyCtx[i], commandBuffers[i])
#endif
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            core::LogError("Failed to record command buffer!");
            std::terminate();
        }
    }
}

void HelloTriangle::CleanupSwapchain()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    vkDestroyPipeline(driver.device, graphicsPipeline_, nullptr);
    vkDestroyPipelineLayout(driver.device, pipelineLayout_, nullptr);
}

void HelloTriangle::RecreateSwapchain()
{
    CreateGraphicsPipeline();
    CreateCommands();
}

}