#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "File.h"
#include "FileSystemObject.h"
#include "RWLock.h"


using FileSystemEntities = std::vector<FileSystemObjectPtr>;


class Directory : public FileSystemObject
{
public:
    Directory(FileSystemObjectPtr parent, const std::string& name, timepoint created, timepoint modified, std::unique_ptr<RWLock::RWLocker>& lock);
    Directory(const std::string& name, timepoint created, timepoint modified, std::unique_ptr<RWLock::RWLocker>& lock);

    const FileSystemEntities& contents(std::unique_ptr<RWLock::RLocker>& lock) const;
    
    void edit(std::unique_ptr<RWLock::RWLocker>& lock);
    void addChild(FileSystemObjectPtr child);

    void visit(FileSystemObjectVisitor& visitor) const override;
    void edit(FileSystemObjectEditor& editor) override;

    FilePtr findFile(const std::string& relativePath);
    ConstFilePtr findFile(const std::string& relativePath) const;

    std::shared_ptr<Directory> findDirectory(const std::string& relativePath);
    std::shared_ptr<Directory> findParent(const std::string& relativePath, std::list<std::string>& remainingPathParts);

    void Create();

    Directory() = delete;
    Directory(const Directory&) = delete;
    Directory& operator=(const Directory&) = delete;
private:
    FilePtr findFile(std::list<std::string>& pathParts);
    ConstFilePtr findFile(std::list<std::string>& pathParts) const;

    std::shared_ptr<Directory> findDirectory(std::list<std::string>& pathParts);

    FileSystemEntities _contents;
    RWLock _lock;
    bool _initializationComplete;
    mutable std::mutex _modificationLock;
    mutable std::condition_variable _condVar;
};

using DirectoryPtr = std::shared_ptr<Directory>;
using ConstDirectoryPtr = std::shared_ptr<const Directory>;