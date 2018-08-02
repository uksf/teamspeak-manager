#pragma once

#include "Common.h"

class IClient {
public:
    virtual void stop() = 0;
    virtual void start() = 0;

    virtual int checkIfBlacklisted(char* name) = 0;

    virtual void procInitaliseClientMaps() = 0;
    virtual void procUpdateServerGroups(std::vector<std::string>) = 0;
    virtual void procAssignServerGroup(std::vector<std::string>) = 0;
    virtual void procUnassignServerGroup(std::vector<std::string>) = 0;
    virtual void procGetServerSnapshot() = 0;
    virtual void procSendMessageToClient(std::vector<std::string>) = 0;
    virtual void procGetOnlineClients() = 0;
    virtual void procShutdown() = 0;

    DECLARE_INTERFACE_MEMBER(STATE, State)
};
