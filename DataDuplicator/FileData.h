#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


struct FileData
{
    uint64_t size;
    std::string name;
    time_t created;
    time_t modified;
    unsigned state;
};


struct DirectorData
{
    std::string name;
    std::vector<DirectorData> directories;
    std::vector<FileData> files;
};