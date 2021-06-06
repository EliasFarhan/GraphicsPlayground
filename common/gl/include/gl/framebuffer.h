#pragma once

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
        DEFAULT = COLOR_ATTACHMENT_0 | DEPTH_RBO
    };

    ~Framebuffer();

    void Create();

    void Reload();

    void Destroy();

    void Bind() const;

    void Clear(const glm::vec3& color);

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

    [[nodiscard]] unsigned int GetColorTexture() const
    { return colorBuffer_; }

    [[nodiscard]] unsigned int GetDepthTexture() const
    { return depthBuffer_; }

    [[nodiscard]] bool IsBinded() const;

    static void CheckFramebuffer(std::string_view file, int line);
private:
    inline static unsigned int currentFramebufferBind_ = 0;
    unsigned int frameBufferType_ = DEFAULT;
    glm::vec<2, int, glm::defaultp> size_;
    unsigned int fbo_ = 0;
    unsigned int colorBuffer_ = 0;
    unsigned int depthRbo_ = 0;
    unsigned int depthBuffer_ = 0;

};

}