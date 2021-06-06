#include <gl/texture.h>

#include "filesystem.h"
#include "log.h"
#include <GL/glew.h>
#include "fmt/core.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#ifdef TRACY_ENABLE

#include <TracyC.h>

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
}
