#pragma once
#include "Common.h"
#include "IMessage.h"
#include "Lockable.h"
#include "IServer.h"
#include <thread>
#include <concurrent_queue.h>

class PipeManager : public IServer, public Lockable {
public:
    PipeManager();
    ~PipeManager();

    void readLoop();
    void sendLoop();
    void initialize() override;
    void shutdown() override;
    void sendMessage(IMessage * message) override;

    DECLARE_MEMBER(HANDLE, PipeHandleRead)
    DECLARE_MEMBER(HANDLE, PipeHandleWrite)
    DECLARE_MEMBER(BOOL, ConnectedWrite)
    DECLARE_MEMBER(BOOL, ConnectedRead)
    DECLARE_MEMBER(BOOL, ShuttingDown)
    DECLARE_MEMBER(std::string, FromPipeName)
    DECLARE_MEMBER(std::string, ToPipeName)

public:
    BOOL getConnected() override { return (this->getConnectedRead() && this->getConnectedWrite()); }
    void setConnected(const BOOL value) override { this->setConnectedRead(value); this->setConnectedWrite(value); }
private:
    Concurrency::concurrent_queue<IMessage *> m_sendQueue;
    std::thread m_readThread;
    std::thread m_sendThread;
};

