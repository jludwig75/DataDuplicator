#pragma once

#include <cstdint>
#include <memory>


class FileHandle
{
public:
    FileHandle(void* handle);
    ~FileHandle();

    size_t read(size_t offset, size_t numberOfBytes, uint8_t* buffer) const;
    size_t write(size_t offset, size_t numberOfBytes, const uint8_t* buffer);

    FileHandle() = delete;
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
private:
    void* _handle;
};

using FileHandlePtr = std::unique_ptr<FileHandle>;
using ConstFileHandlePtr = std::unique_ptr<const FileHandle>;
