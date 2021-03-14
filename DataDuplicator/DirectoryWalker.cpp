#include "DirectoryWalker.h"

#include <algorithm>

#include "OnBlockExit.h"

#include <Windows.h>



namespace
{

std::chrono::system_clock::time_point fileTimeToTimePoint(const FILETIME& ft)
{
    ULARGE_INTEGER liFileTime{ ft.dwLowDateTime, ft.dwHighDateTime };
    time_t ftTime = liFileTime.QuadPart / 10000000ULL - 11644473600ULL;
    return std::chrono::system_clock::from_time_t(ftTime);
}

std::string cleanupName(const std::string& name)
{
    auto clean = name;
    while (clean.back() == '/')
    {
        clean.pop_back();
    }

    std::replace(clean.begin(), clean.end(), '\\', '/');

    return clean;
}

void enumerateDirectory(DirectoryPtr directory)
{
    auto searchPath = directory->path();
    if (searchPath.back() != '/')
    {
        searchPath += '/';
    }
    searchPath += "*";

    WIN32_FIND_DATAA findData;
    auto searchHandle = FindFirstFileExA(searchPath.c_str(), FindExInfoBasic, &findData, FindExSearchNameMatch, NULL, 0);
    if (INVALID_HANDLE_VALUE == searchHandle)
    {
        // TODO: better error handling.
        auto error = GetLastError();
        printf("Error %u searching for files in \"%s\"\n", error, directory->path().c_str());
        return;
    }
    OnBlockExit searchAutoHandle([searchHandle]() { FindClose(searchHandle); });

    do
    {
        std::string objectName = findData.cFileName;
        if (objectName != "." && objectName != "..")
        {
            FileSystemObjectPtr object;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                std::unique_ptr<RWLock::RWLocker> lock;
                auto dir = std::make_shared<Directory>(directory,
                    objectName,
                    fileTimeToTimePoint(findData.ftCreationTime),
                    fileTimeToTimePoint(findData.ftLastWriteTime),
                    lock);

                enumerateDirectory(dir);
                object = dir;
            }
            else
            {
                ULARGE_INTEGER fileSize{ findData.nFileSizeLow, findData.nFileSizeHigh };
                object = std::make_shared<File>(directory,
                    objectName,
                    fileTimeToTimePoint(findData.ftCreationTime),
                    fileTimeToTimePoint(findData.ftLastWriteTime),
                    fileSize.QuadPart);
            }

            directory->addChild(object);
        }
    } while (FindNextFileA(searchHandle, &findData));
    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        // TODO: better error handling.
        auto error = GetLastError();
        printf("Error %u finding next file in \"%s\"\n", error, directory->path().c_str());
        return;
    }
}

}   // namespace

DirectoryWalker::DirectoryWalker()
    :
    _initializationSarted(false)
{
}

DirectoryWalker::~DirectoryWalker()
{
    join();
}

void DirectoryWalker::start(const std::string& scanDirectory, DirectoryPtr& root)
{
    _thread = std::make_unique<std::thread>([this, scanDirectory , &root]() {
        walkDirectoryTree(scanDirectory, root);
        });

    std::unique_lock<std::mutex> lock(_mutex);
    _condVar.wait(lock, [this]() { return _initializationSarted; });
}

void DirectoryWalker::join()
{
    if (_thread && _thread->joinable())
    {
        _thread->join();
    }
}

void DirectoryWalker::walkDirectoryTree(const std::string& scanDirectory, DirectoryPtr& root)
{
    FILETIME created;
    FILETIME modified;
    {
        auto rootHandle = CreateFileA(scanDirectory.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);
        if (INVALID_HANDLE_VALUE == rootHandle)
        {
            // TODO: better error handling.
            auto error = GetLastError();
            printf("Error %u opening directory \"%s\"\n", error, scanDirectory.c_str());
            return;
        }
        OnBlockExit rootAutoHandle([rootHandle]() { CloseHandle(rootHandle); });

        if (!GetFileTime(rootHandle, &created, nullptr, &modified))
        {
            // TODO: better error handling.
            throw GetLastError();
        }
    }

    ULARGE_INTEGER liCreated{ created.dwLowDateTime, created.dwHighDateTime};
    time_t createdTime = liCreated.QuadPart / 10000000ULL - 11644473600ULL;

    ULARGE_INTEGER liModified{ modified.dwLowDateTime, modified.dwHighDateTime};
    time_t modifiedTime = liModified.QuadPart / 10000000ULL - 11644473600ULL;

    std::unique_ptr<RWLock::RWLocker> lock;
    root = std::make_shared<Directory>(cleanupName(scanDirectory),
        fileTimeToTimePoint(created),
        fileTimeToTimePoint(modified),
        lock);

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _initializationSarted = true;
        _condVar.notify_all();
    }

    enumerateDirectory(root);
}
