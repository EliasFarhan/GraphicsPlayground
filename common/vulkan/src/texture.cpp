
#include <vk/texture.h>
#include <log.h>
#include <fmt/core.h>
#include <vk/engine.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace vk
{
void Texture::LoadTexture(std::string_view filename)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const VkDeviceSize imageSize = texWidth * texHeight * STBI_rgb_alpha;

    if (!pixels)
    {
        core::LogError(fmt::format("Failed to load texture image: {}", filename));
        return;
    }

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferMemory;

    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    auto& allocator = engine.GetAllocator();
    engine.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
        stagingBufferMemory);
    void* data;
    vmaMapMemory(allocator, stagingBufferMemory, &data);
    std::memcpy(data, pixels, static_cast<size_t>(imageSize));
    vmaUnmapMemory(allocator, stagingBufferMemory);

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(texWidth);
    imageInfo.extent.height = static_cast<uint32_t>(texHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; //Let the implementation decide
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    //Create Image binds automatically the image to the memory
    if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &textureImage_, &textureImageMemory_, nullptr) != VK_SUCCESS)
    {
        core::LogError("Failed to allocate image memory");
    }

    TransitionImageLayout(textureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    engine.CopyBufferToImage(stagingBuffer, textureImage_, texWidth, texHeight);
    TransitionImageLayout(textureImage_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
}

void Texture::Destroy()
{
    auto& engine = Engine::GetInstance();
    auto& allocator = engine.GetAllocator();
    vmaDestroyImage(allocator, textureImage_, textureImageMemory_);
}

VkImageView Texture::CreateImageView()
{
    auto& engine = Engine::GetInstance();
    auto& driver = engine.GetDriver();
    VkImageView imageView;
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureImage_;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(driver.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        core::LogError("Failed to create texture image view!");
    }
    return imageView;
}

void Texture::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    auto& engine = Engine::GetInstance();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        core::LogError("unsupported layout transition!");
        return;
    }
    VkCommandBuffer commandBuffer = engine.BeginSingleTimeCommands();

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    engine.EndSingleTimeCommands(commandBuffer);
}
}
