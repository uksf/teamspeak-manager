#pragma once

#include "ManagerCommon.h"

class IClient {
public:
    virtual int checkIfBlacklisted(char* name) = 0;

    virtual void procUpdateServerGroups(uint64 clientDBID) = 0;
    virtual void procAssignServerGroup(uint64 clientDBID, uint64 serverGroupID) = 0;
    virtual void procUnassignServerGroup(uint64 clientDBID, uint64 serverGroupID) = 0;
    virtual void procSendMessageToClient(uint64 clientDBID, std::string message) = 0;

    DECLARE_INTERFACE_MEMBER(STATE, State)
};
