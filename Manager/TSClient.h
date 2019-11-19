#pragma once

#include "IClient.h"

class TSClient : public IClient {
public:
    TSClient() = default;
    ~TSClient() = default;

    int checkIfBlacklisted(char* name) override;

    void procUpdateServerGroups(uint64 clientDBID) override;
    void procAssignServerGroup(uint64 clientDBID, uint64 serverGroupID) override;
    void procUnassignServerGroup(uint64 clientDBID, uint64 serverGroupID) override;
    void procSendMessageToClient(uint64 clientDBID, std::string message) override;

    DECLARE_MEMBER(STATE, State)
};

