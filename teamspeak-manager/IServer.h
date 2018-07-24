#pragma once

#include "Common.h"

class IServer
{
public:
    virtual ~IServer() = default;

    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual void sendMessage(IMessage *msg) = 0;
    virtual void handleMessage(unsigned char *data) = 0;
    
    DECLARE_INTERFACE_MEMBER(BOOL, Connected)
};