#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "filesystem.h"

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
    void FreeProgram();
    void CreateDefaultProgram(std::string_view vertexPath, std::string_view fragmentPath);

    void SetFloat(std::string_view uniformName, float f);
    void SetInt(std::string_view uniformName, int i);

    void Bind() const;
private:

    static constexpr unsigned INVALID_SHADER = 0;
    unsigned int program_ = 0;
    std::map<std::string, int> uniformMap_;

    int GetUniformLocation(std::string_view uniformName);
    static unsigned CreateShaderProgram(unsigned vertexShader, unsigned fragmentShader);
    unsigned LoadShader(common::BufferFile&& bufferFile, int shaderType) const;
    static unsigned LoadShader(char* shaderContent, unsigned shaderType);

};

}
