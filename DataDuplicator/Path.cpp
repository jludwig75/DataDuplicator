#include "Path.h"


std::list<std::string> splitPath(const std::string& path)
{
    std::list<std::string> pathParts;

    std::string::size_type start = 0;
    std::string::size_type nextSlash;
    while (start < path.length() && (nextSlash = path.find_first_of('/', start)) != std::string::npos)
    {
        if (nextSlash > start + 1)  // Not a typical split, because we want to collapse consecutive path seprarators
        {
            pathParts.push_back(path.substr(start, nextSlash - start));
        }

        start = nextSlash + 1;
    }

    if (start < path.length())
    {
        pathParts.push_back(path.substr(start));
    }

    return pathParts;
}
