//
// Created by efarhan on 6/5/21.
//

#include "gl/error.h"
#include <GL/glew.h>
#include <fmt/core.h>

namespace gl
{

void CheckError(std::string_view file, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        // Process/log the error.
        switch (err)
        {
            case GL_INVALID_ENUM:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_INVALID_ENUM", file, line));
                break;
            case GL_INVALID_VALUE:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_INVALID_VALUE", file, line));
                break;
            case GL_INVALID_OPERATION:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_INVALID_OPERATION", file, line));
                break;
            case GL_STACK_OVERFLOW:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_STACK_OVERFLOW", file, line));
                break;
            case GL_STACK_UNDERFLOW:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_STACK_UNDERFLOW", file, line));
                break;
            case GL_OUT_OF_MEMORY:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_OUT_OF_MEMORY", file, line));
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_INVALID_FRAMEBUFFER_OPERATION", file, line));
                break;
            case GL_CONTEXT_LOST:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_CONTEXT_LOST", file, line));
                break;
            case GL_TABLE_TOO_LARGE:
                core::LogError(fmt::format("File: {} Line: {} OpenGL: GL_TABLE_TOO_LARGE", file, line));
                break;
            default:
                break;
        }
    }
}
}