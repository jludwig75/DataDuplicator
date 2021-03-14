#include "File.h"

#include "FileSystemObjectVisitor.h"

#include <Windows.h>


File::File(FileSystemObjectPtr parent,
    const std::string& name,
    timepoint created,
    timepoint modified,
    uint64_t size)
        :
    FileSystemObject(parent, name, created, modified),
    _size(size)
{
}

uint64_t File::size() const
{
    return _size;
}

void File::visit(FileSystemObjectVisitor& visitor) const
{
    visitor.onFile(std::static_pointer_cast<const File>(shared_from_this()));
}

void File::edit(FileSystemObjectEditor& editor)
{
    editor.onFile(std::static_pointer_cast<File>(shared_from_this()));
}

ConstFileHandlePtr File::openForRead() const
{
    auto handle = CreateFileA(path().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == handle)
    {
        // TODO: Throw an error?
        return {};
    }

    return std::make_unique<FileHandle>(handle);
}

FileHandlePtr File::openForWrite()
{
    auto handle = CreateFileA(path().c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE == handle)
    {
        // TODO: Throw an error?
        return {};
    }

    return std::make_unique<FileHandle>(handle);
}

void File::Delete()
{
    if (!DeleteFileA(path().c_str()))
    {
        // TODO: better error handling
        throw 1;
    }
}
