#include <iostream>
#include <gl/shader.h>
#include <gl/error.h>
#include <log.h>

#include <GL/glew.h>
#include <fmt/core.h>
#ifdef TRACY_ENABLE

#include "Tracy.hpp"
#include "TracyOpenGL.hpp"

#endif
namespace gl
{
ShaderProgram::~ShaderProgram()
{
    if(program_)
    {
        core::LogWarning("Shader program is not free");
    }
}

void ShaderProgram::Destroy()
{
    if (program_ != 0)
    {
#ifdef TRACY_ENABLE
        ZoneNamedN(shaderDestroy, "Shader Destroy", true);
        TracyGpuNamedZone(shaderDestroyGpu, "Shader Destroy", true);
#endif
        glDeleteProgram(program_);
        program_ = 0;
        glCheckError();
    }
}

unsigned ShaderProgram::LoadShader(core::BufferFile&& bufferFile, int shaderType) const
{
    if (bufferFile.dataBuffer == nullptr)
    {
        core::LogError("Shader file is empty...");
        return INVALID_SHADER;
    }
    return LoadShader(reinterpret_cast<char*>(bufferFile.dataBuffer), shaderType);
}

unsigned ShaderProgram::LoadShader(char* shaderContent, unsigned shaderType)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(shaderLoad, "Shader Load", true);
    TracyGpuNamedZone(shaderDLoadGpu, "Shader Load", true);
#endif
    const GLuint shader = glCreateShader(shaderType);

    glCheckError();


    glShaderSource(shader, 1, &shaderContent, nullptr);
    glCompileShader(shader);
    glCheckError();
    //Check success status of shader compilation
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        core::LogError(fmt::format("[Error] Shader compilation failed with this log:\n{}\nShader content:\n{}",
                                   infoLog,
                                   shaderContent));
        return INVALID_SHADER;
    }
    return shader;
}

void ShaderProgram::CreateDefaultProgram(std::string_view vertexPath, std::string_view fragmentPath)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(shaderProgramCreate, "Shader Program Create", true);
    TracyGpuNamedZone(shaderProgramCreateGpu, "Shader Program Create", true);
#endif
    auto& filesystem = core::FilesystemLocator::get();
    core::BufferFile vertexFile = filesystem.LoadFile(vertexPath);

    const GLuint vertexShader = LoadShader(std::move(vertexFile), GL_VERTEX_SHADER);
    if (vertexShader == INVALID_SHADER)
    {
        core::LogError(fmt::format("[Error] Loading vertex shader: {} unsuccessful", vertexPath));
        return;
    }
    core::BufferFile fragmentFile = filesystem.LoadFile(fragmentPath);

    const GLuint fragmentShader = LoadShader(std::move(fragmentFile), GL_FRAGMENT_SHADER);
    if (fragmentShader == INVALID_SHADER)
    {
        glDeleteShader(vertexShader);
        std::cerr << fmt::format("[Error] Loading fragment shader: {} unsuccessful", fragmentPath) << '\n';
        return;
    }

    glCheckError();
    program_ = CreateShaderProgram(vertexShader, fragmentShader);
    if (program_ == 0)
    {
        std::cerr << fmt::format("[Error] Loading shader program with vertex: {} and fragment {}",
                                 vertexPath, fragmentPath) << '\n';
    }
    glCheckError();
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glCheckError();
}

void ShaderProgram::SetFloat(std::string_view uniformName, float f)
{
    glUniform1f(GetUniformLocation(uniformName), f);
    glCheckError();
}

void ShaderProgram::SetInt(std::string_view uniformName, int i)
{
    glUniform1i(GetUniformLocation(uniformName), i);
    glCheckError();
}

void ShaderProgram::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    glUniform2fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void ShaderProgram::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    glUniform3fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void ShaderProgram::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    glUniform4fv(GetUniformLocation(uniformName), 1, &v[0]);
    glCheckError();
}

void ShaderProgram::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
    glUniformMatrix4fv(GetUniformLocation(uniformName), 1, 0, &mat[0][0]);
    glCheckError();
}

void ShaderProgram::Bind() const
{
    glUseProgram(program_);
    glCheckError();
}

void ShaderProgram::SetTexture(std::string_view uniformName, const Texture& texture, int textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(texture.GetType(), texture.GetName());
    glUniform1i(GetUniformLocation(uniformName), textureUnit);
    glCheckError();

}

void ShaderProgram::SetTexture(std::string_view uniformName,
                               unsigned int textureName, int textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glUniform1i(GetUniformLocation(uniformName), textureUnit);
    glCheckError();
}


int ShaderProgram::GetUniformLocation(std::string_view uniformName)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(uniformLocationTrace, "Get Uniform Location", true);
    TracyGpuNamedZone(uniformLocationGpuTrace, "Get Uniform Location", true);
#endif
    const auto uniformIt = uniformMap_.find(uniformName.data());
    GLint uniformLocation;
    if (uniformIt == uniformMap_.end())
    {
        uniformLocation = glGetUniformLocation(program_, uniformName.data());
        uniformMap_[uniformName.data()] = uniformLocation;
    } else
    {
        uniformLocation = uniformIt->second;
    }
    glCheckError();
    return uniformLocation;
}

unsigned ShaderProgram::CreateShaderProgram(unsigned vertexShader, unsigned fragmentShader)
{
#ifdef TRACY_ENABLE
    ZoneNamedN(createShaderProgram, "Link Shader Program", true);
    TracyGpuNamedZone(createShaderProgramGpu, "Link Shader Program", true);
#endif
    GLuint program = glCreateProgram();
    glCheckError();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glCheckError();
    //Check if shader program was linked correctly
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        core::LogError(fmt::format("[Error] Shader program with vertex {} and fragment {}: LINK_FAILED with infoLog:\n{}",
                                 vertexShader,
                                 fragmentShader,
                                 infoLog));
        return 0;
    }
    return program;
}


}
