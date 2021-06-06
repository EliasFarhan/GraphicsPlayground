#include <filesystem.h>

#include <fmt/core.h>

#include <filesystem>
#include <fstream>

#include "log.h"

#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#endif

namespace fs = std::filesystem;

namespace core
{
BufferFile::~BufferFile()
{
    Destroy();
}

BufferFile::BufferFile(BufferFile&& bufferFile) noexcept
{
    this->dataBuffer = bufferFile.dataBuffer;
    this->dataLength = bufferFile.dataLength;
    bufferFile.dataBuffer = nullptr;
    bufferFile.dataLength = 0;
}

BufferFile& BufferFile::operator=(BufferFile&& bufferFile) noexcept
{
    this->dataBuffer = bufferFile.dataBuffer;
    this->dataLength = bufferFile.dataLength;
    bufferFile.dataBuffer = nullptr;
    bufferFile.dataLength = 0;
    return *this;
}

void BufferFile::Destroy()
{
    if (dataBuffer != nullptr)
    {
        delete[] dataBuffer;
        dataBuffer = nullptr;
        dataLength = 0;
    }
}

Filesystem::Filesystem()
{
    FilesystemLocator::provide(this);
}

BufferFile Filesystem::LoadFile(std::string_view path) const
{
#ifdef TRACY_ENABLE
    ZoneScoped;
#endif
    BufferFile newFile;
    if (FileExists(path))
    {
        std::ifstream is(path.data(), std::ifstream::binary);
        if (!is)
        {
            LogError(fmt::format("[Error] Could not open file: {}  for BufferFile", path));
        } else
        {
            is.seekg(0, is.end);
            newFile.dataLength = is.tellg();
            is.seekg(0, is.beg);
            newFile.dataBuffer = static_cast<unsigned char*>(malloc(newFile.dataLength + 1));
            newFile.dataBuffer[newFile.dataLength] = 0;
            is.read(reinterpret_cast<char*>(newFile.dataBuffer), newFile.dataLength);
            is.close();
        }
    }
    return newFile;
}

bool Filesystem::FileExists(std::string_view path) const
{
    const fs::path p = path;
    return fs::exists(p);
}

bool Filesystem::IsRegularFile(std::string_view path) const
{
    const fs::path p = path;
    return fs::is_regular_file(p);
}

bool Filesystem::IsDirectory(std::string_view path) const
{
    const fs::path p = path;
    return fs::is_directory(p);
}
}
