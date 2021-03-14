#pragma once

#include <cstdint>
#include <memory>

#include "FileHandle.h"
#include "FileSystemObject.h"


class File : public FileSystemObject
{
public:
    File(FileSystemObjectPtr parent, const std::string& name, timepoint created, timepoint modified, uint64_t size);
    uint64_t size() const;

    void visit(FileSystemObjectVisitor& visitor) const override;
    void edit(FileSystemObjectEditor& editor) override;

    ConstFileHandlePtr openForRead() const;
    FileHandlePtr openForWrite();

    void Delete();

    File() = delete;
    File(const File&) = delete;
    File& operator=(const File&) = delete;
private:
    uint64_t _size;
};

using FilePtr = std::shared_ptr<File>;
using ConstFilePtr = std::shared_ptr<const File>;
