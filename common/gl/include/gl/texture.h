#pragma once
#include <string_view>
#include <glm/vec2.hpp>

namespace gl
{    
class Texture
{
public:
    ~Texture();
    void LoadTexture(std::string_view path, 
        int channelsDesired = 0, 
        bool mipmap = true, 
        bool smooth = true,
        bool clamp_wrap = true);
    void Destroy();
    unsigned int GetName() const { return textureName_; }
private:
    unsigned int textureName_ = 0;
    glm::vec2 textureSize_;
};
}
