#include "DataDestination.h"

#include "DataSource.h"
#include "DirectoryWalker.h"


void DataDestination::ReportFiles(ReportFilesRequest& request, ReportFilesResponse& response)
{
    response.success = false;

    try
    {
        if (request.type == ReportFilesRequest::ObjectType::Directory && _localRoot->findDirectory(request.path))
        {
            // We have the file
            response.success = true;
            return;
        }
        auto file = _localRoot->findFile(request.path);
        if (request.type == ReportFilesRequest::ObjectType::File && file)
        {
            if (file->size() == request.size)
            {
                // We have the file
                response.success = true;
                return;
            }
        }

        std::list<std::string> remainingPathParts;
        auto parentDirectory = _localRoot->findParent(request.path, remainingPathParts);
        if (!parentDirectory || remainingPathParts.empty())
        {
            return;
        }

        auto leafName = remainingPathParts.back();
        remainingPathParts.pop_back();
        if (remainingPathParts.size() > 0)
        {
            for (const auto& pathPart : remainingPathParts)
            {
                std::unique_ptr<RWLock::RWLocker> lock;
                auto dir = std::make_shared<Directory>(parentDirectory,
                    pathPart,
                    std::chrono::system_clock::from_time_t(0),
                    std::chrono::system_clock::from_time_t(0),
                    lock);
                dir->Create();
                std::unique_ptr<RWLock::RWLocker> parentLock;
                parentDirectory->edit(parentLock);
                parentDirectory->addChild(dir);
                parentDirectory = dir;
            }
        }

        if (request.type == ReportFilesRequest::ObjectType::File)
        {
            auto file = std::make_shared<File>(parentDirectory, leafName, request.created, request.modified, request.size);
            printf("Copying file \"%s\"\n", file->relativePath().c_str());

            auto fileHandle = file->openForWrite();
            if (!fileHandle)
            {
                // TODO: better error logging/reporting
                printf("Failed to create file \"%s\" for write\n", file->path().c_str());
                return;
            }

            ReadFileDataRequest request;
            request.filePath = file->relativePath();
            ReadFileDataResponse response;
            size_t offset = 0;
            _source->ReadFileData(request, response, [&fileHandle, &offset](bool done, size_t numberOfBytes, uint8_t* data) {
                auto written = fileHandle->write(offset, numberOfBytes, data);
                offset += written;
                return written;
            });
            if (!response.success)
            {
                // Delete the file from disk. We don't know if it was copied completely/correctly
                file->Delete();
            }
            else
            {
                std::unique_ptr<RWLock::RWLocker> parentLock;
                parentDirectory->edit(parentLock);
                parentDirectory->addChild(file);
                response.success = true;
            }
            return;
        }
        else if (request.type == ReportFilesRequest::ObjectType::Directory)
        {
            std::unique_ptr<RWLock::RWLocker> lock;
            auto directory = std::make_shared<Directory>(parentDirectory, leafName, request.created, request.modified, lock);
            printf("Creating directory \"%s\"\n", directory->relativePath().c_str());
            directory->Create();
            std::unique_ptr<RWLock::RWLocker> parentLock;
            parentDirectory->edit(parentLock);
            parentDirectory->addChild(directory);

            response.success = true;
            return;
        }
    }
    catch (...)
    {
    }
}

DataDestination::DataDestination(DataSource* source, const std::string& dataRoot)
    :
    _source(source),
    _dataRoot(dataRoot),
    _runThread(false)
{
    std::unique_ptr<RWLock::RWLocker> lock;
    _localRoot = std::make_shared<Directory>(_dataRoot,
        std::chrono::system_clock::from_time_t(0),  // TODO: get correct times. Actually, we need to walk the tree.
        std::chrono::system_clock::from_time_t(0),
        lock);
}

DataDestination::~DataDestination()
{
    join();
}

void DataDestination::start()
{
    DirectoryWalker walker;

    walker.start(_dataRoot, _localRoot);
    walker.join();

    RegisterDestinationRequest request;
    request.destination = this;

    RegisterDestinationResponse response;
    _source->RegisterDestination(request, response);
    if (!response.success)
    {
        // TODO: better error handling.
        throw 1;
    }

    _runThread = true;
    _thread = std::make_unique<std::thread>([this]() { run();  });
}

void DataDestination::stop()
{
    _runThread = false;
    join();
}

void DataDestination::run()
{
    while (_runThread)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void DataDestination::join()
{
    if (_thread && _thread->joinable())
    {
        _thread->join();
    }
}

