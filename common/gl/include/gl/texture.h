#pragma once

#include <string_view>
#include <vector>
#include <glm/vec2.hpp>
#include "filesystem.h"

namespace gl
{
class Texture
{
public:
    enum TextureFlags : std::uint8_t
    {
        MIPMAP = 1u,
        SMOOTH = 1u << 1u,
        CLAMP_WRAP = 1u << 2u,
        MIRROR_REPEAT_WRAP = 1u << 3u,
        GAMMA_CORRECTION = 1u << 4u,
        FLIP_Y = 1u << 5u,
        DEFAULT = MIPMAP | SMOOTH | CLAMP_WRAP,
    };
    Texture();

    Texture(const Texture& other) = delete;

    Texture(Texture&& other) noexcept;

    Texture& operator=(const Texture& other) = delete;

    Texture& operator=(Texture&& other) noexcept;

    ~Texture();

    void LoadTexture(std::string_view path,
        std::uint8_t textureFlags = DEFAULT,
                     int channelsDesired = 0);

    void LoadCubemap(const std::vector<std::string_view>& paths);

    void Destroy();

    [[nodiscard]] unsigned int GetName() const
    { return textureName_; }

    void CreateWhiteTexture();

    [[nodiscard]] unsigned int GetType() const;
    void SetName(unsigned textureName);
    void SetType(unsigned textureType);

private:
    void LoadCompressedTexture(core::BufferFile&& file);
    unsigned int textureName_ = 0;
    unsigned int textureType_;
    glm::vec2 textureSize_;
};
}
