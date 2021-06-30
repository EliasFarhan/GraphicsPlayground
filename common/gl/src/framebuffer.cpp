//
// Created by efarhan on 6/6/21.
//

#include "GL/glew.h"
#include "gl/framebuffer.h"
#include "log.h"
#include "gl/error.h"
#include "fmt/core.h"

#ifdef TRACY_ENABLE

#include "Tracy.hpp"
#include "TracyOpenGL.hpp"

#endif

namespace gl
{
Framebuffer::~Framebuffer()
{
    if (fbo_ != 0)
    {
        core::LogWarning("Framebuffer is not free!");
    }
}

void Framebuffer::Create()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Create Framebuffer");
#endif
    glCheckError();
    glGenFramebuffers(1, &fbo_);
    Bind();
    glCheckError();

    if (frameBufferType_ & DEPTH_ATTACHMENT)
    {
        if (frameBufferType_ & DEPTH_CUBEMAP)
        {
            glGenTextures(1, &depthBuffer_);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthBuffer_);
            for (unsigned int i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT16,
                             size_.x, size_.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, depthBuffer_, 0);
            glCheckError();
        }
        else
        {
            glGenTextures(1, &depthBuffer_);
            glBindTexture(GL_TEXTURE_2D, depthBuffer_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,
                         size_.x, size_.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                         nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                            GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC,
                            GL_LEQUAL);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer_,
                                   0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glCheckError();
        }
    }
    else if (frameBufferType_ & DEPTH_RBO)
    {
        glGenRenderbuffers(1, &depthRbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRbo_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size_.x,
                              size_.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, depthRbo_);
        glCheckError();
    }
    if (frameBufferType_ & NO_DRAW)
    {
        GLenum drawBuffers = GL_NONE;
        glDrawBuffers(1, &drawBuffers);
        glReadBuffer(GL_NONE);
        glCheckError();
    }
    else if (frameBufferType_ & COLOR_ATTACHMENT_0)
    {
        glGenTextures(colorAttachmentNmb, &colorBuffers_[0]);
        for (int i = 0; i < colorAttachmentNmb; i++)
        {
            glBindTexture(GL_TEXTURE_2D, colorBuffers_[i]);
            glTexImage2D(GL_TEXTURE_2D, 0,
                frameBufferType_ & HDR ? GL_RGBA16F : GL_RGBA8, size_.x,
                size_.y, 0, GL_RGBA,
                frameBufferType_ & HDR ? GL_FLOAT : GL_UNSIGNED_BYTE,
                nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i,
                GL_TEXTURE_2D, colorBuffers_[i], 0);
            glCheckError();
        }
        constexpr std::array<unsigned, MAX_COLOR_ATTACHMENT> attachments {
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT0+1,
            GL_COLOR_ATTACHMENT0+2,
            GL_COLOR_ATTACHMENT0+3,
        };
        glDrawBuffers(colorAttachmentNmb, attachments.data());
    }

    CheckFramebuffer(__FILE__, __LINE__);
    Unbind();
}

void Framebuffer::Reload()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Reload Framebuffer");
#endif
    //Just unbind before Destroy...
    if (fbo_ != 0 && IsBinded())
        Unbind();
    Destroy();
    Create();
}

void Framebuffer::Destroy()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Delete Framebuffer");
#endif
    if (fbo_)
        glDeleteFramebuffers(1, &fbo_);
    if (colorBuffers_[0])
        glDeleteTextures(colorAttachmentNmb, &colorBuffers_[0]);
    if (depthBuffer_)
        glDeleteTextures(1, &depthBuffer_);
    if (depthRbo_)
        glDeleteRenderbuffers(1, &depthRbo_);
    glCheckError();
    fbo_ = 0;
    std::for_each(colorBuffers_.begin(), colorBuffers_.begin() + colorAttachmentNmb, [](auto& colorBuffer) {colorBuffer = 0; });
    depthBuffer_ = 0;
    depthRbo_ = 0;
}

void Framebuffer::Bind() const
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Bind Framebuffer");
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    currentFramebufferBind_ = fbo_;
}

void Framebuffer::Clear(const glm::vec3& color)
{
    glClearColor(color.x * 255, color.y * 255, color.z * 255, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::Unbind()
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Unbind Framebuffer");
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    currentFramebufferBind_ = 0;
}

void Framebuffer::SetType(std::uint32_t newType)
{
    frameBufferType_ = newType;
}

void Framebuffer::SetSize(glm::vec<2, int, glm::defaultp> size)
{
    size_ = size;
}

void Framebuffer::CheckFramebuffer(std::string_view file, int line)
{
#ifdef TRACY_ENABLE
    ZoneScoped;
    TracyGpuZone("Check Framebuffer");
#endif
    const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::string log;
        switch (status)
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                log = "Framebuffer is undefined!";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                log = "Framebuffer is unsupported!";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                log = "Framebuffer has incomplete attachment!";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                log = "Framebuffer has incomplete missing attachment!";
                break;
            default:
                return;
        }
        core::LogError(
                fmt::format("{} in file: {} at line: {}", log, file, line));
    }
}

bool Framebuffer::IsBinded() const
{
    return currentFramebufferBind_ == fbo_;
}

}