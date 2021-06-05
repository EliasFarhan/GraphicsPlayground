#pragma once

#include <string_view>

namespace gl
{
void CheckError(std::string_view file, int line);
}