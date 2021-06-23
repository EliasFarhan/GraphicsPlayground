#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include <glm/ext/matrix_float4x4.hpp>

#include "filesystem.h"
#include "texture.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace gl
{

class ShaderProgram
{
public:
    enum ShaderType : std::uint8_t
    {
        VERTEX = 1u,
        FRAGMENT = 1u << 1u,
        GEOMETRY = 1u << 2u,
        TESSELATION_CONTROL = 1u << 3u,
        TESSELATION_EVALUATION = 1u << 4u,
        COMPUTE = 1u << 5u,
        DEFAULT = VERTEX | FRAGMENT
    };

    ~ShaderProgram();

    void Destroy();

    void CreateDefaultProgram(std::string_view vertexPath, std::string_view fragmentPath);

    void SetFloat(std::string_view uniformName, float f);

    void SetInt(std::string_view uniformName, int i);

    void SetVec2(std::string_view uniformName, glm::vec2 v);

    void SetVec3(std::string_view uniformName, glm::vec3 v);

    void SetVec4(std::string_view uniformName, glm::vec4 v);

    void Bind() const;

    void SetTexture(std::string_view uniformName, const Texture& texture, int textureUnit);
    void SetTexture(std::string_view uniformName, unsigned int textureName, int textureUnit);

    void SetMat4(std::string_view uniformName, const glm::mat4& mat);

private:

    static constexpr unsigned INVALID_SHADER = 0;
    unsigned int program_ = 0;
    std::unordered_map<std::string, int> uniformMap_;

    int GetUniformLocation(std::string_view uniformName);

    static unsigned CreateShaderProgram(unsigned vertexShader, unsigned fragmentShader);

    unsigned LoadShader(core::BufferFile&& bufferFile, int shaderType) const;

    static unsigned LoadShader(char* shaderContent, unsigned shaderType);

};

}
