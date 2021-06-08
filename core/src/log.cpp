#include <log.h>
#include "spdlog/spdlog.h"

namespace core
{


void LogDebug(const std::string_view msg)
{
    spdlog::info(msg);
}

void LogWarning(const std::string_view msg)
{
    spdlog::warn(msg);
}

void LogError(const std::string_view msg)
{

    spdlog::error(msg);
}
}
