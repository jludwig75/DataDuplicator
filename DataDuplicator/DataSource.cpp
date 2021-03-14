#include "DataSource.h"

#include "DataDestination.h"
#include "DirectoryWalker.h"


DataSource::DataSource(const std::string& dataRoot)
    :
    _destination(nullptr),
    _dataRoot(dataRoot),
    _runThread(false)
{
}

DataSource::~DataSource()
{
    join();
}

void DataSource::start()
{
    DirectoryWalker walker;

    walker.start(_dataRoot, _root);

    _runThread = true;
    _thread = std::make_unique<std::thread>([this]() { run(); });
}

void DataSource::stop()
{
    _runThread = false;
    join();
}

void reportDirectorTree(ConstDirectoryPtr directory, DataDestination* destination)
{
    std::unique_ptr<RWLock::RLocker> lock;

    for (auto object : directory->contents(lock))
    {
        FileSystemObjectVisitor(
            [destination](ConstDirectoryPtr dir) {
                ReportFilesRequest request;
                request.type = ReportFilesRequest::ObjectType::Directory;
                request.path = dir->relativePath();
                request.created = dir->created();
                request.modified = dir->modified();

                ReportFilesResponse response;
                destination->ReportFiles(request, response);
                // Check for errors
                if (!response.success)
                {
                    // TODO: better error logging/reporting
                    printf("destination failure on ReportFiles for directory \"%s\"\n", dir->relativePath().c_str());
                }
                else
                {
                    // Only report the contents if we could report the parent.
                    reportDirectorTree(dir, destination);
                }
            },
            [destination](ConstFilePtr file) {
                ReportFilesRequest request;
                request.type = ReportFilesRequest::ObjectType::File;
                request.path = file->relativePath();
                request.created = file->created();
                request.modified = file->modified();
                request.size = file->size();

                ReportFilesResponse response;
                destination->ReportFiles(request, response);
                // Check for errors
                if (!response.success)
                {
                    // TODO: better error logging/reporting
                    printf("destination failure on ReportFiles for file \"%s\"\n", file->relativePath().c_str());
                }
            }).visit(object);
    }
}

void DataSource::run()
{
    bool reportedFiles = false;
    while (_runThread && !reportedFiles)
    {
        // TODO: This will look a lot different when it runs continuously
        // For now we just report once.
        if (_destination && !reportedFiles)
        {
            reportDirectorTree(_root, _destination);
            printf("Done reporting files\n");
            reportedFiles = true;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void DataSource::join()
{
    if (_thread && _thread->joinable())
    {
        _thread->join();
    }
}

void DataSource::RegisterDestination(const RegisterDestinationRequest& request, RegisterDestinationResponse& response)
{
    if (_destination != nullptr)
    {
        response.success = false;
        return;
    }

    _destination = request.destination;
    response.success = true;
}

void DataSource::ReadFileData(const ReadFileDataRequest& request, ReadFileDataResponse& response, WriteStreamFunction receiveChuck) const
{
    // Set to true when completed successfully
    response.success = false;
    auto file = _root->findFile(request.filePath);
    if (!file)
    {
        return;
    }

    auto fileHandle = file->openForRead();
    if (!fileHandle)
    {
        // TODO: better error logging/reporting
        printf("Failed to open file \"%s\" for read\n", file->path().c_str());
        return;
    }

    std::vector<uint8_t> dataBuffer(1024 * 1024, 0);
    auto bytesToCopy = file->size();
    decltype(file->size()) offset = 0;
    while (bytesToCopy > 0)
    {
        try
        {

            auto bytesRead = fileHandle->read(offset, dataBuffer.size(), dataBuffer.data());
            if (bytesRead == 0)
            {
                // We're not at the end of the file yet, so we should have read something.
                return;
            }
            auto bytesWritten = receiveChuck(false, bytesRead, dataBuffer.data());
            if (bytesWritten == 0)
            {
                // Should have written something.
                return;
            }

            offset += bytesWritten;
            bytesToCopy -= bytesWritten;
        }
        catch (...)
        {
            return;
        }
    }

    response.success = true;
}
