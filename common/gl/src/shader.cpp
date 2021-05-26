#include <iostream>
#include <gl/shader.h>

#include "gl/glew.h"
#include <fmt/core.h>

namespace gl
{
    ShaderProgram::~ShaderProgram()
    {
        FreeProgram();
    }

    void ShaderProgram::FreeProgram()
    {
        if (program_ != 0)
        {
            glDeleteProgram(program_);
            program_ = 0;
        }
    }

    unsigned ShaderProgram::LoadShader(common::BufferFile&& bufferFile, int shaderType) const
    {
        if (bufferFile.dataBuffer == nullptr)
            return INVALID_SHADER;
        return LoadShader(reinterpret_cast<char*>(bufferFile.dataBuffer), shaderType);
    }

    unsigned ShaderProgram::LoadShader(char* shaderContent, unsigned shaderType)
    {
        const GLuint shader = glCreateShader(shaderType);



        glShaderSource(shader, 1, &shaderContent, nullptr);
        glCompileShader(shader);
        //Check success status of shader compilation
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << fmt::format("[Error] Shader compilation failed with this log:\n{}\nShader content:\n{}",
                infoLog,
                shaderContent) << '\n';
            return INVALID_SHADER;
        }
        return shader;
    }

    void ShaderProgram::CreateDefaultProgram(std::string_view vertexPath, std::string_view fragmentPath)
    {
        auto& filesystem = common::FilesystemLocator::get();
        common::BufferFile vertexFile = filesystem.LoadFile(vertexPath);

        const GLuint vertexShader = LoadShader(std::move(vertexFile), GL_VERTEX_SHADER);
        if (vertexShader == INVALID_SHADER)
        {
            std::cerr << fmt::format("[Error] Loading vertex shader: {} unsuccessful", vertexPath) << '\n';
            return;
        }
        common::BufferFile fragmentFile = filesystem.LoadFile(fragmentPath);

        const GLuint fragmentShader = LoadShader(std::move(fragmentFile), GL_FRAGMENT_SHADER);
        if (fragmentShader == INVALID_SHADER)
        {
            glDeleteShader(vertexShader);
            std::cerr << fmt::format("[Error] Loading fragment shader: {} unsuccessful", fragmentPath) << '\n';
            return;
        }

        program_ = CreateShaderProgram(vertexShader, fragmentShader);
        if (program_ == 0)
        {
            std::cerr << fmt::format("[Error] Loading shader program with vertex: {} and fragment {}",
                vertexPath, fragmentPath) << '\n';
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void ShaderProgram::SetFloat(std::string_view uniformName, float f)
    {
        glUniform1f(GetUniformLocation(uniformName), f);
    }

    void ShaderProgram::SetInt(std::string_view uniformName, int i)
    {
        glUniform1i(GetUniformLocation(uniformName), i);
    }

    void ShaderProgram::Bind() const
    {
        glUseProgram(program_);
    }

    int ShaderProgram::GetUniformLocation(std::string_view uniformName)
    {
        const auto uniformIt = uniformMap_.find(uniformName.data());
        GLint uniformLocation;
        if (uniformIt == uniformMap_.end())
        {
            uniformLocation = glGetUniformLocation(program_, uniformName.data());
            uniformMap_[uniformName.data()] = uniformLocation;
        }
        else
        {
            uniformLocation = uniformIt->second;
        }
        return uniformLocation;
    }

    unsigned ShaderProgram::CreateShaderProgram(unsigned vertexShader, unsigned fragmentShader)
    {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
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
