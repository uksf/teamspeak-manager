#pragma once

#include "Common.h"
#include "Macros.h"
#include "IMessage.h"
#include "IServer.h"
#include "Lockable.h"
#include <concurrent_queue.h>

class CommandServer : public IServer, public Lockable {
public:
    CommandServer();
    explicit CommandServer(ID id);
    ~CommandServer();

    void initialize() override;
    void shutdown() override;

    void sendMessage(IMessage* msg) override;
    void handleMessage(unsigned char* data);

    void release();

DECLARE_MEMBER(char *, CommandId)
DECLARE_MEMBER(BOOL, Connected)
DECLARE_MEMBER(ID, Id)
private:
    concurrency::concurrent_queue<IMessage *> mInboundMessages;
};
