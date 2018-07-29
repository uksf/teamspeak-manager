#pragma once

#include "IClient.h"

class TSClient : public IClient {
public:
    TSClient() = default;
    ~TSClient() = default;

    void stop() override;
    void start() override;

    int checkIfBlacklisted(char* name);

    void procUpdateServerGroups(std::vector<std::string>) override;
    void procAssignServerGroup(std::vector<std::string>) override;
    void procUnassignServerGroup(std::vector<std::string>) override;
    void procGetServerSnapshot() override;
    void procSendMessageToClient(std::vector<std::string>) override;

    void finishSnapshotForClient(anyID clientID, uint64 clientDatabaseID) override;

    DECLARE_MEMBER(STATE, State)
    DECLARE_MEMBER_PRIVATE(anyID, LastSnapshotClient)
};

