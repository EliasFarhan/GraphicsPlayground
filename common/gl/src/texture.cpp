#include <gl/texture.h>

#include "filesystem.h"
#include "log.h"
#include <GL/glew.h>
#include "fmt/core.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

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

    //stbi_set_flip_vertically_on_load(true);
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
                &channelNb, channelsDesired);
    }
    /*if (extension == ".hdr")
    {
        //data = stbi_loadf(filename.data(), &width, &height, &reqComponents, 0);
        data = stbi_loadf_from_memory((unsigned char*) (textureFile.dataBuffer),
                                      textureFile.dataLength, &width, &height, &reqComponents, 0);
    }*/

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
    CheckError(__FILE__, __LINE__);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    clamp_wrap ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    clamp_wrap ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    smooth ? GL_LINEAR : GL_NEAREST);
    CheckError(__FILE__, __LINE__);
    if (mipmap)
    {

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        smooth ? GL_LINEAR_MIPMAP_LINEAR
                               : GL_NEAREST_MIPMAP_LINEAR);
        CheckError(__FILE__, __LINE__);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        smooth ? GL_LINEAR : GL_NEAREST);
        CheckError(__FILE__, __LINE__);
    }
    switch (channelNb)
    {
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, imageWidth, imageHeight,
                         0,
                         GL_RED, GL_UNSIGNED_BYTE, imageData);
            break;
        case 2:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, imageWidth, imageHeight,
                         0,
                         GL_RG, GL_UNSIGNED_BYTE, imageData);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight,
                         0,
                         GL_RGB, GL_UNSIGNED_BYTE, imageData);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight,
                         0,
                         GL_RGBA, GL_UNSIGNED_BYTE, imageData);
            break;
        default:
            break;
    }

    CheckError(__FILE__, __LINE__);
    //For HDR
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, imageWidth, imageHeight, 0, GL_RGB, GL_FLOAT, imageData);

    if (mipmap)
    {
#ifdef TRACY_ENABLE
        TracyGpuNamedZone(mipmapGeneration, "Mipmap Generation", true);
#endif
        glGenerateMipmap(GL_TEXTURE_2D);
        CheckError(__FILE__, __LINE__);
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
        CheckError(__FILE__, __LINE__);
    }
}

void Texture::CreateWhiteTexture()
{
    glGenTextures(1, &textureName_);
    glBindTexture(GL_TEXTURE_2D, textureName_);
    unsigned char white[] = { 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CheckError(__FILE__, __LINE__);
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
    glGenTextures(1, &textureName_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureName_);
    textureType_ = GL_TEXTURE_CUBE_MAP;


    for (unsigned int i = 0; i < paths.size(); i++)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(loadTexture, "Texture Loading", true);
        TracyGpuNamedZone(loadTextureGpu, "Texture Loading", true);
#endif
        auto& filesystem = core::FilesystemLocator::get();
        if (!filesystem.FileExists(paths[i]))
        {
            core::LogError(fmt::format("[Error] Texture: {} does not exist", paths[i]));
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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData
            );
        }
        else
        {
            core::LogError(fmt::format("Cubemap tex failed to load at path: {}", paths[i]));
        }
        free(imageData);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    CheckError(__FILE__, __LINE__);
}

unsigned int Texture::GetType() const
{
    return textureType_;
}
}
