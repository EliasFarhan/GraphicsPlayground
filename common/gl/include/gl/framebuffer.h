#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace gl
{
class Framebuffer
{
public:
    enum Type : unsigned int
    {
        COLOR_ATTACHMENT_0 = 1u,
        DEPTH_RBO = 1u << 1u,
        NO_DRAW = 1u << 2u,
        DEPTH_STENCIL_RBO = 1u << 3u,
        HDR = 1u << 4u,
        DEPTH_ATTACHMENT = 1u << 5u,
        DEPTH_STENCIL_ATTACHMENT = 1u << 6u,
        DEPTH_CUBEMAP = 1u << 7u,
        DEFAULT = COLOR_ATTACHMENT_0 | DEPTH_RBO
    };

    ~Framebuffer();

    void Create();

    void Reload();

    void Destroy();

    void Bind() const;

    void Clear(const glm::vec3& color);
    void SetColorAttachmentNmb(std::size_t nmb) { colorAttachmentNmb = nmb; }

    static void Unbind();

    /*
     * Please Use gl::Framebuffer::Type (for example Framebuffer::NO_DRAW)
     * Always Reload() to take effect
     */
    void SetType(std::uint32_t);

    /*
     * Always Reload() to take effect
     */
    void SetSize(glm::vec<2, int, glm::defaultp> size);

    [[nodiscard]] unsigned int GetColorTexture(int i = 0) const
    { return colorBuffers_[i]; }

    [[nodiscard]] unsigned int GetDepthTexture() const
    { return depthBuffer_; }

    [[nodiscard]] bool IsBinded() const;

    static void CheckFramebuffer(std::string_view file, int line);

    [[nodiscard]] auto GetSize() const { return size_; }

private:
    inline static unsigned int currentFramebufferBind_ = 0;
    unsigned int frameBufferType_ = DEFAULT;
    glm::vec<2, int, glm::defaultp> size_;
    unsigned int fbo_ = 0;
    static constexpr int MAX_COLOR_ATTACHMENT = 4;
    int colorAttachmentNmb = 1;
    std::array<unsigned int, MAX_COLOR_ATTACHMENT> colorBuffers_{};
    unsigned int depthRbo_ = 0;
    unsigned int depthBuffer_ = 0;

};

}