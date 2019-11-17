#pragma once

#include "ManagerCommon.h"

class IClient {
public:
    virtual int checkIfBlacklisted(char* name) = 0;

    virtual void procUpdateServerGroups(std::vector<std::string>) = 0;
    virtual void procAssignServerGroup(std::vector<std::string>) = 0;
    virtual void procUnassignServerGroup(std::vector<std::string>) = 0;
    virtual void procSendMessageToClient(std::vector<std::string>) = 0;

    DECLARE_INTERFACE_MEMBER(STATE, State)
};
