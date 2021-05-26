#pragma once
#include <string_view>
#include <service_locator.h>

namespace core
{

    /**
     * \brief Non-copyable RAII structure that represents a file buffered in RAM
     */
    struct BufferFile
    {
        BufferFile() = default;
        ~BufferFile();
        BufferFile(BufferFile&& bufferFile) noexcept;
        BufferFile& operator=(BufferFile&& bufferFile) noexcept;
        BufferFile(const BufferFile&) = delete;
        BufferFile& operator= (const BufferFile&) = delete;

        unsigned char* dataBuffer = nullptr;
        size_t dataLength = 0;
        void Destroy();

    };

    class FilesystemInterface
    {
    public:
        virtual ~FilesystemInterface() = default;
        [[nodiscard]] virtual BufferFile LoadFile(std::string_view path) const = 0;
        [[nodiscard]] virtual bool FileExists(std::string_view) const = 0;
        [[nodiscard]] virtual bool IsRegularFile(std::string_view) const = 0;
        [[nodiscard]] virtual bool IsDirectory(std::string_view) const = 0;
    };

    class NullFilesystem : public FilesystemInterface
    {
        [[nodiscard]] BufferFile LoadFile(std::string_view path) const override { return {}; }

        [[nodiscard]] bool FileExists(std::string_view view) const override { return false; }

        [[nodiscard]] bool IsRegularFile(std::string_view view) const override { return false; }

        [[nodiscard]] bool IsDirectory(std::string_view view) const override { return false; }
    };

    class Filesystem : public FilesystemInterface
    {
    public:
        Filesystem();
        [[nodiscard]] BufferFile LoadFile(std::string_view path) const override;

        [[nodiscard]] bool FileExists(std::string_view path) const override;

        [[nodiscard]] bool IsRegularFile(std::string_view path) const override;

        [[nodiscard]] bool IsDirectory(std::string_view path) const override;

    };
    using FilesystemLocator = Locator<FilesystemInterface, NullFilesystem>;
}
