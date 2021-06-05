#pragma once

#include <string_view>

namespace core
{

void LogDebug(std::string_view msg);

void LogWarning(std::string_view msg);

void LogError(std::string_view msg);
}