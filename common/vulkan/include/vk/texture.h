#pragma once

#include <string_view>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vk
{
class Texture
{
public:
    void LoadTexture(std::string_view filename);
    void Destroy();
    VkImageView CreateImageView();
    static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

private:
    VkImage textureImage_{};
    VmaAllocation textureImageMemory_{};
};
}