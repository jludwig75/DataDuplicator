#pragma once

#include <mutex>
#include <vector>


class RWLock
{
public:
    RWLock();
    class RWLocker
    {
    public:
        RWLocker(RWLock& lock)
            :
            lock(lock)
        {
            lock.acquireWrite();
        }
        ~RWLocker()
        {
            lock.releaseWrite();
        }
        RWLock& lock;
    };
    class RLocker
    {
    public:
        RLocker(const RWLock& lock)
            :
            lock(lock)
        {
            lock.acquireRead();
        }
        ~RLocker()
        {
            lock.releaseRead();
        }
        const RWLock& lock;
    };
    bool acquired() const;
    bool acquiredForWrite() const;
private:
    void acquireRead() const;
    void acquireWrite();
    void releaseRead() const;
    void releaseWrite();
    mutable  bool _acuiredForWrite;
    mutable std::vector<std::pair<std::thread::id, size_t>> _owners;
    mutable size_t _lockCount;
    mutable std::mutex _mutex;
    mutable std::condition_variable _condVar;
};
