#pragma once

#include <string_view>
#include <vector>
#include <glm/vec2.hpp>

namespace gl
{
class Texture
{
public:
    Texture();

    Texture(const Texture& other) = delete;

    Texture(Texture&& other) noexcept;

    Texture& operator=(const Texture& other) = delete;

    Texture& operator=(Texture&& other) noexcept;

    ~Texture();

    void LoadTexture(std::string_view path,
                     int channelsDesired = 0,
                     bool mipmap = true,
                     bool smooth = true,
                     bool clamp_wrap = true);

    void LoadCubemap(const std::vector<std::string_view>& paths);

    void Destroy();

    [[nodiscard]] unsigned int GetName() const
    { return textureName_; }

    void CreateWhiteTexture();

    [[nodiscard]] unsigned int GetType() const;

private:
    void LoadCompressedTexture(core::BufferFile&& file);
    unsigned int textureName_ = 0;
    unsigned int textureType_;
    glm::vec2 textureSize_;
};
}
