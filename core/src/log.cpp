#include <log.h>
#include <iostream>

namespace core
{
void LogDebug(std::string_view msg)
{
    std::cout << "[Debug]" << msg << '\n';
}

void LogWarning(std::string_view msg)
{
    std::cout << "[Warning]" << msg << '\n';
}

void LogError(std::string_view msg)
{

    std::cout << "[Error]" << msg << '\n';
}
}
