#include <gl/texture.h>

#include "filesystem.h"
#include "log.h"
#include <GL/glew.h>
#include "fmt/core.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "gli/gli.hpp"

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#include "TracyOpenGL.hpp"
#endif

namespace gl
{
Texture::~Texture()
{
    if (textureName_)
    {
        core::LogWarning("Texture is not free");
    }
}

void
Texture::LoadTexture(std::string_view path, int channelsDesired, bool mipmap,
                     bool smooth, bool clamp_wrap)
{

#ifdef TRACY_ENABLE
    ZoneNamedN(loadTexture, "Texture Loading", true);
    TracyGpuNamedZone(loadTextureGpu, "Texture Loading", true);
#endif
    auto& filesystem = core::FilesystemLocator::get();
    if (!filesystem.FileExists(path))
    {
        core::LogError(fmt::format("[Error] Texture: {} does not exist", path));
        return;
    }
    core::BufferFile textureFile = filesystem.LoadFile(path);

    const auto extension = core::FilesystemInterface::GetExtension(path);
    bool hdr = false;
    if (extension == ".hdr")
    {
        hdr = true;
    }
    else if (extension == ".ktx" || extension == ".dds")
    {
        LoadCompressedTexture(std::move(textureFile));
        return;
    }
    int imageWidth, imageHeight;
    int channelNb;
    stbi_uc* imageData = nullptr;
    float* hdrImageData = nullptr;
    if (hdr)
    {
        hdrImageData = stbi_loadf_from_memory(
                static_cast<unsigned char*>(textureFile.dataBuffer),
                textureFile.dataLength, &imageWidth,
                &imageHeight, &channelNb, channelsDesired);
    }
    else
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(stbLoad, "STB Load", true);
#endif
        imageData = stbi_load_from_memory(
                textureFile.dataBuffer,
                static_cast<int>(textureFile.dataLength),
                &imageWidth,
                &imageHeight,
                &channelNb, channelsDesired);
    }

    textureFile.Destroy();
    if (imageData == nullptr)
    {
        core::LogDebug(fmt::format("[Error] Texture: cannot load {}", path));
        return;
    }
#ifdef TRACY_ENABLE
    ZoneNamedN(gpuUpload, "GPU Upload", true)
    TracyGpuNamedZone(uploadTextureGpu, "GPU Upload", true);
#endif
    unsigned int texture;
    glGenTextures(1, &texture);
    glCheckError();

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    clamp_wrap ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    clamp_wrap ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    smooth ? GL_LINEAR : GL_NEAREST);
    glCheckError();
    if (mipmap)
    {

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        smooth ? GL_LINEAR_MIPMAP_LINEAR
                               : GL_NEAREST_MIPMAP_LINEAR);
        glCheckError();
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        smooth ? GL_LINEAR : GL_NEAREST);
        glCheckError();
    }
    if (hdr)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, imageWidth, imageHeight, 0,
                     GL_RGB, GL_FLOAT, hdrImageData);
    }
    else
    {
        switch (channelNb)
        {
            case 1:
            {
#ifdef TRACY_ENABLE
                TracyGpuNamedZone(textureRUpload, "Texture RED Upload", true);
#endif
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, imageWidth, imageHeight,
                             0,
                             GL_RED, GL_UNSIGNED_BYTE, imageData);
                break;
            }
            case 2:
            {
#ifdef TRACY_ENABLE
                TracyGpuNamedZone(textureRGUpload, "Texture RG Upload", true);
#endif
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, imageWidth, imageHeight,
                             0,
                             GL_RG, GL_UNSIGNED_BYTE, imageData);
                break;
            }
            case 3:
            {
#ifdef TRACY_ENABLE
                TracyGpuNamedZone(textureRGBUpload, "Texture RGB Upload", true);
#endif
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight,
                             0,
                             GL_RGB, GL_UNSIGNED_BYTE, imageData);
                break;
            }
            case 4:
            {
#ifdef TRACY_ENABLE
                TracyGpuNamedZone(textureRGBAUpload, "Texture RGBA Upload", true);
#endif
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight,
                             0,
                             GL_RGBA, GL_UNSIGNED_BYTE, imageData);
                break;
            }
            default:
                break;
        }
    }

    glCheckError();
    if (mipmap)
    {
#ifdef TRACY_ENABLE
        TracyGpuNamedZone(mipmapGeneration, "Mipmap Generation", true);
#endif
        glGenerateMipmap(GL_TEXTURE_2D);
        glCheckError();
    }
    textureSize_ = glm::vec2(imageWidth, imageHeight);
    free(imageData);
    textureName_ = texture;

}

void Texture::Destroy()
{
    if (textureName_ != 0)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(textureDestroy, "Texture Destroy", true);
        TracyGpuNamedZone(textureDestroyGpu, "Texture Destroy", true);
#endif
        glDeleteTextures(1, &textureName_);
        textureName_ = 0;
        glCheckError();
    }
}

void Texture::CreateWhiteTexture()
{
    glGenTextures(1, &textureName_);
    glBindTexture(GL_TEXTURE_2D, textureName_);
    unsigned char white[] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCheckError();
}

Texture::Texture(Texture&& other) noexcept
{
    textureName_ = other.textureName_;
    other.textureName_ = 0;
    textureSize_ = other.textureSize_;
    textureType_ = other.textureType_;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    textureName_ = other.textureName_;
    other.textureName_ = 0;
    textureSize_ = other.textureSize_;
    textureType_ = other.textureType_;
    return *this;
}

Texture::Texture() : textureType_(GL_TEXTURE_2D), textureSize_(glm::vec2())
{

}

void Texture::LoadCubemap(const std::vector<std::string_view>& paths)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(loadTexture, "Cubemap Texture Loading", true);
    TracyGpuNamedZone(loadTextureGpu, "Cubemap Texture Loading", true);
#endif
    glGenTextures(1, &textureName_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureName_);
    textureType_ = GL_TEXTURE_CUBE_MAP;


    for (unsigned int i = 0; i < paths.size(); i++)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(loadFaceTexture, "Cubemap Texture Face Loading", true);
        TracyGpuNamedZone(loadFaceTextureGpu, "Cubemap Texture Face Loading", true);
#endif
        auto& filesystem = core::FilesystemLocator::get();
        if (!filesystem.FileExists(paths[i]))
        {
            core::LogError(fmt::format("[Error] Texture: {} does not exist",
                                       paths[i]));
            return;
        }
        core::BufferFile textureFile = filesystem.LoadFile(paths[i]);

        int imageWidth, imageHeight;
        int channelNb;
        stbi_uc* imageData = nullptr;
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(stbLoad, "STB Load", true);
#endif
            imageData = stbi_load_from_memory(
                    textureFile.dataBuffer,
                    static_cast<int>(textureFile.dataLength),
                    &imageWidth,
                    &imageHeight,
                    &channelNb, 0);
        }
        textureFile.Destroy();
        if (imageData != nullptr)
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(uploadFaceTexture, "Cubemap Texture Face Uploading", true);
            TracyGpuNamedZone(uploadFaceTextureGpu, "Cubemap Texture Face Uploading",
                              true);
#endif
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, imageData
            );
        }
        else
        {
            core::LogError(fmt::format("Cubemap tex failed to load at path: {}",
                                       paths[i]));
        }
        free(imageData);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glCheckError();
}

unsigned int Texture::GetType() const
{
    return textureType_;
}

void Texture::LoadCompressedTexture(core::BufferFile&& textureFile)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(loadTexture, "KTX Texture Loading", true);
    TracyGpuNamedZone(loadTextureGpu, " KTX Texture Loading", true);
#endif
    gli::gl glProfile(gli::gl::PROFILE_GL33);

    auto texture = gli::load(
            reinterpret_cast<const char*>(textureFile.dataBuffer),
            textureFile.dataLength);
    if (texture.empty())
    {
        core::LogError("Could not load texture with GLI");
        return;
    }
    textureFile.Destroy();
    const gli::gl::format format = glProfile.translate(texture.format(),
                                                       texture.swizzles());

    GLenum target = glProfile.translate(texture.target());

    glm::tvec3<GLsizei> extent{texture.extent()};
    core::LogDebug(fmt::format(
            "Texture format: {}, texture target {}, is compressed {}, layers nmb: {}, faces nmb: {}, extends: {},{}",
            (int) texture.format(),
            (int) texture.target(),
            is_compressed(texture.format()),
            texture.layers(),
            texture.faces(),
            extent.x, extent.y));

    glGenTextures(1, &textureName_);
    glBindTexture(target, textureName_);

    glCheckError();
    glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL,
                    static_cast<GLint>(texture.levels() - 1));
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
    glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);
    glCheckError();
    glTexStorage2D(target, static_cast<GLint>(texture.levels()),
                   format.Internal, extent.x, extent.y);
    textureType_ = target;
    glCheckError();
    for (std::size_t face = 0; face < texture.faces(); ++face)
    {
        for (std::size_t level = 0; level < texture.levels(); ++level)
        {
#ifdef TRACY_ENABLE
            ZoneNamedN(loadFaceTexture, "KTX Face Loading", true);
            TracyGpuNamedZone(loadFaceTextureGpu, " KTX Face Loading", true);
#endif
            target = gli::is_target_cube(texture.target()) ?
                     GL_TEXTURE_CUBE_MAP_POSITIVE_X + face :
                     target;
            glm::tvec3<GLsizei> levelExtent(texture.extent(level));
            if (gli::is_compressed(texture.format()))
            {
                glCompressedTexSubImage2D(
                        target, static_cast<GLint>(level), 0, 0, levelExtent.x,
                        levelExtent.y,
                        format.Internal,
                        static_cast<GLsizei>(texture.size(level)),
                        texture.data(0, face, level));
            }
            else
            {
                glTexSubImage2D(
                        target, static_cast<GLint>(level), 0, 0, levelExtent.x,
                        levelExtent.y,
                        format.External, format.Type,
                        texture.data(0, face, level));
            }
            glCheckError();
        }
    }
    glCheckError();
}
}
