#include "FileHandle.h"

#include <Windows.h>


FileHandle::FileHandle(void* handle)
    :
    _handle(handle)
{
}

FileHandle::~FileHandle()
{
    CloseHandle(_handle);
}

size_t FileHandle::read(size_t offset, size_t numberOfBytes, uint8_t* buffer) const
{
    LARGE_INTEGER off;
    off.QuadPart = offset;
    LARGE_INTEGER newOff;
    if (!SetFilePointerEx(_handle, off, &newOff, FILE_BEGIN))
    {
        // TODO: better error handling.
        throw GetLastError();
    }
    if (off.QuadPart != newOff.QuadPart)
    {
        // TODO: better error handling.
        throw 1;
    }
    
    DWORD bytesRead;
    if (!ReadFile(_handle, buffer, static_cast<DWORD>(numberOfBytes), &bytesRead, NULL))
    {
        // TODO: better error handling.
        throw GetLastError();
    }

    return bytesRead;
}

size_t FileHandle::write(size_t offset, size_t numberOfBytes, const uint8_t* buffer)
{
    LARGE_INTEGER off;
    off.QuadPart = offset;
    LARGE_INTEGER newOff;
    if (!SetFilePointerEx(_handle, off, &newOff, FILE_BEGIN))
    {
        // TODO: better error handling.
        throw GetLastError();
    }
    if (off.QuadPart != newOff.QuadPart)
    {
        // TODO: better error handling.
        throw 1;
    }

    DWORD bytesWritten;
    if (!WriteFile(_handle, buffer, static_cast<DWORD>(numberOfBytes), &bytesWritten, NULL))
    {
        // TODO: better error handling.
        throw GetLastError();
    }

    return bytesWritten;
}
