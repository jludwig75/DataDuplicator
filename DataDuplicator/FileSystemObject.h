#pragma once

#include <chrono>
#include <memory>
#include <string>


using timepoint = std::chrono::system_clock::time_point;

class FileSystemObject;
using FileSystemObjectPtr = std::shared_ptr<FileSystemObject>;
using ConstFileSystemObjectPtr = std::shared_ptr<const FileSystemObject>;


class FileSystemObjectVisitor;
class FileSystemObjectEditor;


class FileSystemObject : public std::enable_shared_from_this<FileSystemObject>
{
public:
    FileSystemObject(FileSystemObjectPtr parent, const std::string& name, timepoint created, timepoint modified);
    std::string name() const;
    std::string path() const;
    std::string relativePath() const;
    timepoint created() const;
    timepoint modified() const;

    virtual void visit(FileSystemObjectVisitor& visitor) const = 0;
    virtual void edit(FileSystemObjectEditor& editor) = 0;

    FileSystemObject() = delete;
    FileSystemObject(const FileSystemObject&) = delete;
    FileSystemObject& operator=(const FileSystemObject&) = delete;
private:
    FileSystemObjectPtr _parent;
    const std::string _name;
    timepoint _created;
    timepoint _modified;
};

