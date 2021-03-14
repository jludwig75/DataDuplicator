#include "FileSystemObject.h"


FileSystemObject::FileSystemObject(FileSystemObjectPtr parent,
    const std::string& name,
    timepoint created,
    timepoint modified)
        :
    _parent(parent),
    _name(name),
    _created(created),
    _modified(modified)
{
}

std::string FileSystemObject::name() const
{
    return _name;
}

std::string FileSystemObject::path() const
{
    if (_parent)
    {
        return _parent->path() + "/" + name();
    }

    return name();
}

std::string FileSystemObject::relativePath() const
{
    if (_parent)
    {
        return _parent->relativePath() + "/" + name();
    }

    return "";
}

timepoint FileSystemObject::created() const
{
    return _created;
}

timepoint FileSystemObject::modified() const
{
    return _modified;
}

