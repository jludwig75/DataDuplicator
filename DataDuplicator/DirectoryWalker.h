#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Directory.h"
#include "File.h"
#include "FileSystemObjectVisitor.h"

class DirectoryWalker
{
public:
    DirectoryWalker();
    ~DirectoryWalker();
    void start(const std::string& scanDirectory, DirectoryPtr& root);
    void join();
private:
    void walkDirectoryTree(const std::string& scanDirectory, DirectoryPtr& root);

    std::unique_ptr<std::thread> _thread;
    bool _initializationSarted;
    std::mutex _mutex;
    std::condition_variable _condVar;
};