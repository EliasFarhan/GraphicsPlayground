#include <log.h>
#include <cstring>
#include <vk/utility.h>
#include "filesystem.h"
#include "hello_index_buffer.h"

namespace vk
{
void HelloIndexBuffer::Init()
{
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateGraphicsPipeline();
    CreateCommands();
}

void HelloIndexBuffer::Update([[maybe_unused]] core::seconds dt)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(inputBufferUpdate, "Input Buffer Update", true);
#endif
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
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
    if (vkQueueSubmit(driver.graphicsQueue, 1, &submitInfo, renderer.inFlightFences[renderer.currentFrame]) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to submit draw command buffer!");
        std::terminate();
    }
}

void HelloIndexBuffer::Destroy()
{
    auto& engine = Engine::GetInstance();
    auto& allocator = engine.GetAllocator();
    vmaDestroyBuffer(allocator, vertexBuffer_, vertexAllocation_);
    vmaDestroyBuffer(allocator, indexBuffer_, indexAllocation_);
    CleanupSwapchain();
}

void HelloIndexBuffer::OnEvent([[maybe_unused]]SDL_Event& event)
{
}

void HelloIndexBuffer::DrawImGui()
{
}

void HelloIndexBuffer::CleanupSwapchain()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    vkDestroyPipeline(driver.device, graphicsPipeline_, nullptr);
    vkDestroyPipelineLayout(driver.device, pipelineLayout_, nullptr);
}

void HelloIndexBuffer::RecreateSwapchain()
{
    CreateGraphicsPipeline();
    CreateCommands();
}

void HelloIndexBuffer::CreateGraphicsPipeline()
{
    core::LogDebug("Create Graphics Pipeline");
    const auto& filesystem = core::FilesystemLocator::get();
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();

    core::BufferFile vertexShaderFile = filesystem.LoadFile(
        "data/shaders/02_hello_input_buffer/triangle.vert.spv");
    core::BufferFile fragmentShaderFile = filesystem.LoadFile(
        "data/shaders/02_hello_input_buffer/triangle.frag.spv");

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

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

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

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil; // Optional
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

void HelloIndexBuffer::CreateCommands()
{
    core::LogDebug("Create Commands");
    auto& engine = Engine::GetInstance();
    auto& renderer = engine.GetRenderer();
    auto& swapchain = engine.GetSwapchain();

    auto& commandBuffers = renderer.commandBuffers;
    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            core::LogError("Failed to begin recording command buffer!");
            std::terminate();
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderer.renderPass;
        renderPassInfo.framebuffer = renderer.framebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain.extent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

        VkBuffer vertexBuffers[] = {vertexBuffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(renderer.commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(renderer.commandBuffers[i], indexBuffer_, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);


        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            core::LogError("Failed to record command buffer!");
            std::terminate();
        }
    }
}

void HelloIndexBuffer::CreateVertexBuffer()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    auto& engine = Engine::GetInstance();
    auto& allocator = engine.GetAllocator();
    const auto bufferSize = sizeof(vertices_[0]) * vertices_.size();
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    engine.CreateBuffer(bufferSize,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer,
                        stagingAllocation);
    void* data;
    vmaMapMemory(allocator, stagingAllocation, &data);
    std::memcpy(data, vertices_.data(), vertices_.size() * sizeof(Vertex));
    vmaUnmapMemory(allocator, stagingAllocation);
    engine.CreateBuffer(bufferSize,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        vertexBuffer_, vertexAllocation_);
    engine.CopyBuffer(stagingBuffer, vertexBuffer_, bufferSize);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

void HelloIndexBuffer::CreateIndexBuffer()
{
    auto& engine = Engine::GetInstance();
    auto& allocator = engine.GetAllocator();
    VkDeviceSize bufferSize = sizeof(indices_[0]) * indices_.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    engine.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                        stagingAllocation);

    void* data;
    vmaMapMemory(allocator, stagingAllocation, &data);
    std::memcpy(data, indices_.data(), (size_t)bufferSize);
    vmaUnmapMemory(allocator, stagingAllocation);

    engine.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer_,
                        indexAllocation_);

    engine.CopyBuffer(stagingBuffer, indexBuffer_, bufferSize);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

VkVertexInputBindingDescription
HelloIndexBuffer::Vertex::GetBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2>
HelloIndexBuffer::Vertex::GetAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}
}
