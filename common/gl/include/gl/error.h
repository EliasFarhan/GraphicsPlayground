#pragma once

#include <string_view>

namespace gl
{
void CheckError(std::string_view file, int line);
#define glCheckError() CheckError(__FILE__, __LINE__)
}