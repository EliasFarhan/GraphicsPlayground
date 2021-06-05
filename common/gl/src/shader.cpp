#include <iostream>
#include <gl/shader.h>

#include <GL/glew.h>
#include <fmt/core.h>

namespace gl
{
ShaderProgram::~ShaderProgram()
{
    Destroy();
}

void ShaderProgram::Destroy()
{
    if (program_ != 0)
    {
        glDeleteProgram(program_);
        program_ = 0;
        CheckError(__FILE__, __LINE__);
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
    const GLuint shader = glCreateShader(shaderType);

    CheckError(__FILE__, __LINE__);


    glShaderSource(shader, 1, &shaderContent, nullptr);
    glCompileShader(shader);
    CheckError(__FILE__, __LINE__);
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

    CheckError(__FILE__, __LINE__);
    program_ = CreateShaderProgram(vertexShader, fragmentShader);
    if (program_ == 0)
    {
        std::cerr << fmt::format("[Error] Loading shader program with vertex: {} and fragment {}",
                                 vertexPath, fragmentPath) << '\n';
    }
    CheckError(__FILE__, __LINE__);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::SetFloat(std::string_view uniformName, float f)
{
    glUniform1f(GetUniformLocation(uniformName), f);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::SetInt(std::string_view uniformName, int i)
{
    glUniform1i(GetUniformLocation(uniformName), i);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::SetVec2(std::string_view uniformName, glm::vec2 v)
{
    glUniform2fv(GetUniformLocation(uniformName), 1, &v[0]);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::SetVec3(std::string_view uniformName, glm::vec3 v)
{
    glUniform3fv(GetUniformLocation(uniformName), 1, &v[0]);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::SetVec4(std::string_view uniformName, glm::vec4 v)
{
    glUniform4fv(GetUniformLocation(uniformName), 1, &v[0]);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::SetMat4(std::string_view uniformName, const glm::mat4& mat)
{
    glUniformMatrix4fv(GetUniformLocation(uniformName), 1, 0, &mat[0][0]);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::Bind() const
{
    glUseProgram(program_);
    CheckError(__FILE__, __LINE__);
}

void ShaderProgram::SetTexture(std::string_view uniformName, const Texture& texture, int textureUnit)
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture.GetName());
    glUniform1i(GetUniformLocation(uniformName), textureUnit);
    CheckError(__FILE__, __LINE__);

}

int ShaderProgram::GetUniformLocation(std::string_view uniformName)
{
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
    CheckError(__FILE__, __LINE__);
    return uniformLocation;
}

unsigned ShaderProgram::CreateShaderProgram(unsigned vertexShader, unsigned fragmentShader)
{
    GLuint program = glCreateProgram();
    CheckError(__FILE__, __LINE__);
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    CheckError(__FILE__, __LINE__);
    //Check if shader program was linked correctly
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << fmt::format("[Error] Shader program with vertex {} and fragment {}: LINK_FAILED with infoLog:\n{}",
                                 vertexShader,
                                 fragmentShader,
                                 infoLog) << '\n';
        return 0;
    }
    return program;
}

}
