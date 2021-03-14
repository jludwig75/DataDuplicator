
#include "Directory.h"

#include <cassert>
#include <list>

#include "FileSystemObjectVisitor.h"
#include "Path.h"

#include <Windows.h>


Directory::Directory(FileSystemObjectPtr parent,
    const std::string& name,
    timepoint created,
    timepoint modified,
    std::unique_ptr<RWLock::RWLocker>& lock)
        :
    FileSystemObject(parent, name, created, modified),
    _initializationComplete(false)
{
    lock = std::make_unique<RWLock::RWLocker>(_lock);
}

Directory::Directory(const std::string& name,
    timepoint created,
    timepoint modified,
    std::unique_ptr<RWLock::RWLocker>& lock)
        :
    Directory(nullptr, name, created, modified, lock)
{
}

void Directory::edit(std::unique_ptr<RWLock::RWLocker>& lock)
{
    lock = std::make_unique<RWLock::RWLocker>(_lock);
}

void Directory::addChild(FileSystemObjectPtr child)
{
    assert(_lock.acquiredForWrite());

    _contents.push_back(child);
}

const FileSystemEntities& Directory::contents(std::unique_ptr<RWLock::RLocker>& lock) const
{
    lock = std::make_unique<RWLock::RLocker>(_lock);

    return _contents;
}

void Directory::visit(FileSystemObjectVisitor& visitor) const
{
    RWLock::RLocker lock(_lock);

    visitor.onDirectory(std::static_pointer_cast<const Directory>(shared_from_this()));
}

void Directory::edit(FileSystemObjectEditor& editor)
{
    RWLock::RWLocker lock(_lock);

    editor.onDirectory(std::static_pointer_cast<Directory>(shared_from_this()));
}

FilePtr Directory::findFile(const std::string& relativePath)
{
    auto pathParts = splitPath(relativePath);

    if (pathParts.empty())
    {
        // This would be the root which is not a file
        return {};
    }

    return findFile(pathParts);
}

ConstFilePtr Directory::findFile(const std::string& relativePath) const
{
    auto pathParts = splitPath(relativePath);

    if (pathParts.empty())
    {
        // This would be the root which is not a file
        return {};
    }

    return findFile(pathParts);
}


FilePtr Directory::findFile(std::list<std::string>& pathParts)
{
    auto* dir = this;
    while (dir)
    {
        std::unique_ptr<RWLock::RLocker> lock;
        for (auto object : dir->contents(lock))
        {
            FilePtr foundFile;
            FileSystemObjectEditor(
                [&pathParts, &foundFile](DirectoryPtr directory) {
                    if (pathParts.size() > 1 && pathParts.front() == directory->name())
                    {
                        pathParts.pop_front();
                        foundFile = directory->findFile(pathParts);
                    }
                },
                [&pathParts, &foundFile](FilePtr file) {
                    if (pathParts.size() == 1 && pathParts.front() == file->name())
                    {
                        foundFile = file;
                    }
                }
            ).edit(object);

            if (foundFile)
            {
                return foundFile;
            }
        }
        return {};
    }
    return {};
}

ConstFilePtr Directory::findFile(std::list<std::string>& pathParts) const
{
    auto* dir = this;
    while (dir)
    {
        std::unique_ptr<RWLock::RLocker> lock;
        for (auto object : dir->contents(lock))
        {
            ConstFilePtr foundFile;
            FileSystemObjectVisitor(
                [&pathParts, &foundFile](ConstDirectoryPtr directory) {
                    if (pathParts.size() > 1 && pathParts.front() == directory->name())
                    {
                        foundFile = directory->findFile(pathParts);
                    }
                },
                [&pathParts, &foundFile](ConstFilePtr file) {
                    if (pathParts.size() == 1 && pathParts.front() == file->name())
                    {
                        foundFile = file;
                    }
                }
            ).visit(object);

            if (foundFile)
            {
                return foundFile;
            }
        }
    }
    return {};
}

std::shared_ptr<Directory> Directory::findDirectory(const std::string& relativePath)
{
    auto pathParts = splitPath(relativePath);

    if (pathParts.empty())
    {
        // This would be the root which is not a file
        return {};
    }

    return findDirectory(pathParts);
}

std::shared_ptr<Directory> Directory::findDirectory(std::list<std::string>& pathParts)
{
    if (pathParts.empty())
    {
        return std::static_pointer_cast<Directory>(shared_from_this());
    }
    auto* dir = this;
    while (dir)
    {
        std::unique_ptr<RWLock::RLocker> lock;
        for (auto object : dir->contents(lock))
        {
            DirectoryPtr foundDirectory;
            bool failed = false;
            FileSystemObjectEditor(
                [&pathParts, &foundDirectory](DirectoryPtr directory) {
                    if (pathParts.size() > 0 && pathParts.front() == directory->name())
                    {
                        pathParts.pop_front();
                        foundDirectory = directory->findDirectory(pathParts);
                    }
                },
                [&pathParts, &failed](FilePtr file) {
                    // Ignore
                }
                ).edit(object);

                if (foundDirectory)
                {
                    return foundDirectory;
                }

                if (failed)
                {
                    return {};
                }
        }

        return{};
    }
    return {};
}

std::shared_ptr<Directory> Directory::findParent(const std::string& relativePath, std::list<std::string>& remainingPathParts)
{
    auto pathParts = splitPath(relativePath);
    if (pathParts.empty())
    {
        return std::static_pointer_cast<Directory>(shared_from_this());
    }

    auto parentDir = std::static_pointer_cast<Directory>(shared_from_this());
    while (!pathParts.empty())
    {
        auto& pathPart = pathParts.front();
        auto dir = parentDir->findDirectory(pathPart);
        if (!dir)
        {
            remainingPathParts = pathParts;
            return parentDir;
        }

        pathParts.pop_front();
        parentDir = dir;
    }

    return parentDir;
}


void Directory::Create()
{
    if (!CreateDirectoryA(path().c_str(), NULL))
    {
        // TODO: better error handling
        throw 1;
    }
}
