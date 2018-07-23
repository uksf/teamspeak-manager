#pragma once
#include <mutex>

class Lockable {
    std::recursive_mutex m_lockable_mutex;
public:
    virtual ~Lockable() = default;

    void lock() {
        m_lockable_mutex.lock();
    };

    void unlock() {
        m_lockable_mutex.unlock();
    };
};
