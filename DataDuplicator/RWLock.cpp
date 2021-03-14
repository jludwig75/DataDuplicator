#include "RWLock.h"

#include <cassert>


RWLock::RWLock()
    :
    _acuiredForWrite(false),
    _lockCount(0)
{
}

bool RWLock::acquired() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    assert(!_acuiredForWrite || _lockCount == 1);

    return _lockCount > 0;
}

bool RWLock::acquiredForWrite() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    assert(!_acuiredForWrite || _owners.size() == 1);

    return _acuiredForWrite;
}

void RWLock::acquireRead() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    auto owner = std::find_if(_owners.begin(), _owners.end(), [](auto& entry) { return entry.first == std::this_thread::get_id(); });
    if (owner != _owners.end())
    {
        assert(_lockCount > 0);
        assert(owner->second > 0);
        owner->second++;
        _lockCount++;
        return;
    }
    assert(!_acuiredForWrite || _owners.size() == 1);
    _condVar.wait(lock, [this]() { return !_acuiredForWrite; });

    auto threadId = std::this_thread::get_id();
    owner = std::find_if(_owners.begin(), _owners.end(), [threadId](auto& entry) { return entry.first == threadId; });
    if (owner != _owners.end())
    {
        owner->second++;
    }
    else
    {
        _owners.emplace_back(threadId, 1);
    }

    _lockCount++;
}

void RWLock::acquireWrite()
{
    std::unique_lock<std::mutex> lock(_mutex);
    auto owner = std::find_if(_owners.begin(), _owners.end(), [](auto& entry) { return entry.first == std::this_thread::get_id(); });
    if (owner != _owners.end())
    {
        assert(_lockCount > 0);
        assert(owner->second > 0);
        owner->second++;
        _lockCount++;
        _acuiredForWrite = true;
        return;
    }
    assert(!_acuiredForWrite || _owners.size() == 1);
    _condVar.wait(lock, [this]() { return _lockCount == 0; });
    assert(!_acuiredForWrite);
    assert(_lockCount == 0 && _owners.size() == 0);

    auto threadId = std::this_thread::get_id();
    owner = std::find_if(_owners.begin(), _owners.end(), [threadId](auto& entry) { return entry.first == threadId; });
    if (owner != _owners.end())
    {
        owner->second++;
    }
    else
    {
        _owners.emplace_back(threadId, 1);
    }

    _lockCount++;
    _acuiredForWrite = true;
}

void RWLock::releaseRead() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    assert(_owners.size() > 0);
    assert(_lockCount > 0);   // caller precondition

    auto threadId = std::this_thread::get_id();
    auto owner = std::find_if(_owners.begin(), _owners.end(), [threadId](auto& entry) { return entry.first == threadId; });
    assert(owner != _owners.end());
    owner->second--;
    if (owner->second == 0)
    {
        _owners.erase(owner);
    }

    _lockCount--;

    if (_lockCount == 0)
    {
        _acuiredForWrite = false;
        lock.unlock();
        _condVar.notify_all();
    }
}

void RWLock::releaseWrite()
{
    std::unique_lock<std::mutex> lock(_mutex);
    assert(_acuiredForWrite);   // caller precondition
    assert(_owners.size() == 1);
    assert(_lockCount > 0);

    auto threadId = std::this_thread::get_id();
    auto owner = std::find_if(_owners.begin(), _owners.end(), [threadId](auto& entry) { return entry.first == threadId; });
    assert(owner != _owners.end());
    owner->second--;
    if (owner->second == 0)
    {
        _owners.erase(owner);
    }

    _lockCount--;

    if (_lockCount == 0)
    {
        _acuiredForWrite = false;
        lock.unlock();
        _condVar.notify_all();
    }
}
