#pragma once

#include <memory>
#include <string>
#include <thread>

#include "Directory.h"


class DataSource;

// TODO: This goes in a proto file for a gRPC service
struct ReportFilesRequest
{
    enum class ObjectType
    {
        Directory,
        File
    };
    std::string path;
    ObjectType type;
    std::chrono::system_clock::time_point created;
    std::chrono::system_clock::time_point modified;
    uint64_t size{};
};

struct ReportFilesResponse
{
    bool success;
};

class DataDestination
{
public:
    // API used by source
    // TODO: put a gRPC server on this
    void ReportFiles(ReportFilesRequest& request, ReportFilesResponse& response);

    DataDestination(DataSource* source, const std::string& dataRoot);
    ~DataDestination();
    void start();
    void stop();
private:
    void run();
    void join();

    DataSource* _source;
    const std::string _dataRoot;
    std::unique_ptr<std::thread> _thread;
    bool _runThread;
    DirectoryPtr _localRoot;
    DirectoryPtr _RemoteRoot;
};

