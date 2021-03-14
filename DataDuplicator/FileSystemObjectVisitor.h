#pragma once

#include <functional>

#include "FileSystemObject.h"


#include "Directory.h"
#include "File.h"


class FileSystemObjectVisitor
{
public:
    using OnDirectory = std::function<void(ConstDirectoryPtr directory)>;
    using OnFile = std::function<void(ConstFilePtr directory)>;

    FileSystemObjectVisitor(OnDirectory onDirectory, OnFile onFile)
        :
        _onDirectory(onDirectory),
        _onFile(onFile)
    {
    }
    void visit(ConstFileSystemObjectPtr object)
    {
        object->visit(*this);
    }
    void onDirectory(ConstDirectoryPtr directory)
    {
        _onDirectory(directory);
    }
    void onFile(ConstFilePtr file)
    {
        _onFile(file);
    }

    FileSystemObjectVisitor() = delete;
    FileSystemObjectVisitor(const FileSystemObjectVisitor&) = delete;
    FileSystemObjectVisitor& operator=(const FileSystemObjectVisitor&) = delete;
private:
    OnDirectory _onDirectory;
    OnFile _onFile;
};

class FileSystemObjectEditor
{
public:
    using OnDirectory = std::function<void(DirectoryPtr directory)>;
    using OnFile = std::function<void(FilePtr directory)>;

    FileSystemObjectEditor(OnDirectory onDirectory, OnFile onFile)
        :
        _onDirectory(onDirectory),
        _onFile(onFile)
    {
    }
    void edit(FileSystemObjectPtr object)
    {
        object->edit(*this);
    }
    void onDirectory(DirectoryPtr directory)
    {
        _onDirectory(directory);
    }
    void onFile(FilePtr file)
    {
        _onFile(file);
    }

    FileSystemObjectEditor() = delete;
    FileSystemObjectEditor(const FileSystemObjectEditor&) = delete;
    FileSystemObjectEditor& operator=(const FileSystemObjectEditor&) = delete;
private:
    OnDirectory _onDirectory;
    OnFile _onFile;
};
