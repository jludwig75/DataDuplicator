#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "Directory.h"


class DataDestination;

// TODO: This goes in a proto file for a gRPC service
struct RegisterDestinationRequest
{
    DataDestination* destination;
    // TODO: For gRPC this will be an address and port
};

struct RegisterDestinationResponse
{
    bool success{ false };
};


struct ReadFileDataRequest
{
    std::string filePath;
};

struct ReadFileDataResponse
{
    bool success{ false };
};


class DataSource
{
public:
    DataSource(const std::string& dataRoot);
    ~DataSource();

    void start();
    void stop();
    
    // API used by destination.
    // TODO: put a gRPC server on this
    void RegisterDestination(const RegisterDestinationRequest& request, RegisterDestinationResponse& response);
    using WriteStreamFunction = std::function<size_t(bool done, size_t numberOfBytes, uint8_t* data)>;
    void ReadFileData(const ReadFileDataRequest& request, ReadFileDataResponse& response, WriteStreamFunction receiveChuck) const;

    DataSource() = delete;
    DataSource(const DataSource&) = delete;
    DataSource& operator=(const DataSource&) = delete;
private:
    void run();
    void join();

    DataDestination* _destination;
    const std::string _dataRoot;
    std::unique_ptr<std::thread> _thread;
    bool _runThread;
    DirectoryPtr _root;
};
