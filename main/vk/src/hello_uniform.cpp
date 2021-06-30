#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "hello_uniform.h"
#include <vulkan/vulkan.h>
#include <vk/engine.h>
#include <log.h>
#include <filesystem.h>
#include <vk/utility.h>

namespace vk
{
void vk::HelloUniform::Init()
{
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSetLayout();
    CreateDescriptorSets();
    CreateGraphicsPipeline();
    CreateCommands();
}

void vk::HelloUniform::Update(core::seconds dt)
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& renderer = engine.GetRenderer();
   

    UpdateUniformBuffers(dt);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        renderer.imageAvailableSemaphores[renderer.currentFrame]
    };
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer.commandBuffers[renderer.imageIndex];

    VkSemaphore signalSemaphores[] = {
        renderer.renderFinishedSemaphores[renderer.currentFrame]
    };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(driver.graphicsQueue, 1, &submitInfo,
                      renderer.inFlightFences[renderer.currentFrame]) !=
        VK_SUCCESS)
    {
        core::LogError("Failed to submit draw command buffer!");
        std::terminate();
    }
}

void vk::HelloUniform::Destroy()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& allocator = engine.GetAllocator();
    vmaDestroyBuffer(allocator, vertexBuffer_, vertexAllocation_);
    vmaDestroyBuffer(allocator, indexBuffer_, indexAllocation_);
    CleanupSwapchain();
    vkDestroyDescriptorSetLayout(driver.device, descriptorSetLayout_, nullptr);
}

void vk::HelloUniform::OnEvent([[maybe_unused]] SDL_Event& event)
{
}

void vk::HelloUniform::DrawImGui()
{
}

void vk::HelloUniform::CleanupSwapchain()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();
    auto& allocator = engine.GetAllocator();
    vkDestroyPipeline(driver.device, graphicsPipeline_, nullptr);
    vkDestroyPipelineLayout(driver.device, pipelineLayout_, nullptr);
    for (std::size_t i = 0; i < swapchain.imageCount; i++)
    {
        vmaDestroyBuffer(allocator, uniformBuffers_[i],
                         uniformBuffersAllocations_[i]);
    }

    vkDestroyDescriptorPool(driver.device, descriptorPool_, nullptr);
}

void vk::HelloUniform::RecreateSwapchain()
{
    CreateGraphicsPipeline();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommands();
}

void vk::HelloUniform::CreateGraphicsPipeline()
{
    core::LogDebug("Create Graphics Pipeline");
    const auto& filesystem = core::FilesystemLocator::get();
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();

    core::BufferFile vertexShaderFile = filesystem.LoadFile(
        "data/shaders/05_hello_uniform/triangle.vert.spv");
    core::BufferFile fragmentShaderFile = filesystem.LoadFile(
        "data/shaders/05_hello_uniform/triangle.frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertexShaderFile,
                                                         driver.device);
    VkShaderModule fragShaderModule = CreateShaderModule(fragmentShaderFile,
                                                         driver.device);

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

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo,
        fragShaderStageInfo
    };

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
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
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
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(driver.device, &pipelineLayoutInfo, nullptr,
                               &pipelineLayout_) !=
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


    if (vkCreateGraphicsPipelines(driver.device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, nullptr,
                                  &graphicsPipeline_) != VK_SUCCESS)
    {
        core::LogError("Failed to create graphics pipeline!\n");
        std::terminate();
    }
    vkDestroyShaderModule(driver.device, vertShaderModule, nullptr);
    vkDestroyShaderModule(driver.device, fragShaderModule, nullptr);
}

void vk::HelloUniform::CreateCommands()
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


        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) !=
            VK_SUCCESS)
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

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphicsPipeline_);

        VkBuffer vertexBuffers[] = {vertexBuffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(renderer.commandBuffers[i], 0, 1,
                               vertexBuffers, offsets);
        vkCmdBindIndexBuffer(renderer.commandBuffers[i], indexBuffer_, 0,
                             VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(commandBuffers[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout_, 0, 1, &descriptorSets_[i],
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffers[i],
                         static_cast<uint32_t>(indices_.size()), 1, 0, 0,
                         0);

        vkCmdEndRenderPass(commandBuffers[i]);


        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
        {
            core::LogError("Failed to record command buffer!");
            std::terminate();
        }
    }
}

void vk::HelloUniform::CreateVertexBuffer()
{
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

void vk::HelloUniform::CreateIndexBuffer()
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

void HelloUniform::CreateDescriptorSetLayout()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(driver.device, &layoutInfo, nullptr,
                                    &descriptorSetLayout_) != VK_SUCCESS)
    {
        core::LogError("Failed to create descriptor set layout!");
        std::terminate();
    }
}

void HelloUniform::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    auto& engine = Engine::GetInstance();
    auto& swapchain = engine.GetSwapchain();
    uniformBuffers_.resize(swapchain.imageCount);
    uniformBuffersAllocations_.resize(swapchain.imageCount);

    for (std::size_t i = 0; i < swapchain.imageCount; i++)
    {
        engine.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            uniformBuffers_[i], uniformBuffersAllocations_[i]);
    }
}

void HelloUniform::UpdateUniformBuffers(core::seconds duration)
{
    auto& engine = Engine::GetInstance();
    auto& renderer = engine.GetRenderer();
    auto& swapchain = engine.GetSwapchain();
    auto& allocator = engine.GetAllocator();
    UniformBufferObject ubo{};
    dt_ += duration.count();

    ubo.model = glm::rotate(glm::mat4(1.0f),
                            dt_ * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain.extent.width /
                                (float)swapchain.extent.height,
                                0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    void* data;
    vmaMapMemory(allocator, uniformBuffersAllocations_[renderer.imageIndex],
                 &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(allocator,
                   uniformBuffersAllocations_[renderer.imageIndex]);
}

void HelloUniform::CreateDescriptorPool()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapchain.imageCount);
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapchain.imageCount);
    if (vkCreateDescriptorPool(driver.device, &poolInfo, nullptr,
                               &descriptorPool_) != VK_SUCCESS)
    {
        core::LogError("Failed to create descriptor pool!");
        std::terminate();
    }
}

void HelloUniform::CreateDescriptorSets()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& swapchain = engine.GetSwapchain();
    std::vector<VkDescriptorSetLayout> layouts(swapchain.imageCount,
                                               descriptorSetLayout_);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool_;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapchain.imageCount);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets_.resize(swapchain.imageCount);
    if (vkAllocateDescriptorSets(driver.device, &allocInfo,
                                 descriptorSets_.data()) != VK_SUCCESS)
    {
        core::LogError("Failed to allocate descriptor sets!");
        std::terminate();
    }
    for (std::size_t i = 0; i < swapchain.imageCount; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers_[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets_[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional
        vkUpdateDescriptorSets(driver.device, 1, &descriptorWrite, 0, nullptr);
    }
}


VkVertexInputBindingDescription
HelloUniform::Vertex::GetBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2>
HelloUniform::Vertex::GetAttributeDescriptions()
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
