#pragma once

#include "Common.h"
#include "Lockable.h"
#include <concurrent_queue.h>
#include <thread>

template <typename T>
class Worker : public Lockable {
public:
    Worker() {
		{
			std::lock_guard<std::mutex> lock(m_lockable_mutex);
			this->Worker<T>::setRunning(false);
		}
    }

    ~Worker() = default;

    void startWorker() {
		{
			std::lock_guard<std::mutex> lock(m_lockable_mutex);
			this->setShuttingDown(false);
			this->m_processQueue.clear();
			this->workerThread = std::thread(&Worker::workerThreadFunction, this);
			this->setRunning(true);
		}
    }

    void stopWorker() {
		{
			std::lock_guard<std::mutex> lock(m_lockable_mutex);
			this->setShuttingDown(true);
			this->setRunning(false);
			if (this->workerThread.joinable()) {
				this->workerThread.join();
			}
			this->m_processQueue.clear();
			this->setShuttingDown(false);
		}
    }

    void workerThreadFunction() {
        T item;
        while (!this->getShuttingDown()) {
			{
				std::lock_guard<std::mutex> lock(m_lockable_mutex);
				if (this->m_processQueue.try_pop(item)) {
					this->exProcessItem(item);
				}
			}
            Sleep(1);
        }
    }

    virtual void exProcessItem(T) = 0;
DECLARE_MEMBER(BOOL, ShuttingDown)
DECLARE_MEMBER(BOOL, Running)
    std::thread workerThread;
    Concurrency::concurrent_queue<T> m_processQueue;
};
