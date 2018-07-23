#pragma once

#include "Common.h"
#include "Lockable.h"
#include <concurrent_queue.h>
#include <thread>

template <typename T>
class EntrantWorker : public Lockable {
public:
    EntrantWorker() {
        this->EntrantWorker<T>::setRunning(false);
    }

    ~EntrantWorker() { }

    void startWorker() {
        LOCK(this);
        this->setShuttingDown(false);
        this->m_processQueue.clear();
        this->workerThread = std::thread(&EntrantWorker::exWorkerThread, this);
        this->setRunning(true);
        UNLOCK(this);
    }

    void stopWorker() {
        this->setShuttingDown(true);
        this->setRunning(false);
        if (this->workerThread.joinable()) {
            this->workerThread.join();
        }
        LOCK(this);
        this->m_processQueue.clear();
        UNLOCK(this);
        this->setShuttingDown(false);
    }

    void exWorkerThread() {
        T item;
        while (!this->getShuttingDown()) {
            LOCK(this);
            if (this->m_processQueue.try_pop(item)) {
                this->exProcessItem(item);
            }
            UNLOCK(this);
            Sleep(1);
        }
    }

    virtual void exProcessItem(T) = 0;
    DECLARE_MEMBER(BOOL, ShuttingDown)
    DECLARE_MEMBER(BOOL, Running)
    std::thread workerThread;
    Concurrency::concurrent_queue<T> m_processQueue;
};
