#pragma once

#include "IClient.h"

class TSClient : public IClient {
public:
    TSClient() = default;
    ~TSClient() = default;

    int checkIfBlacklisted(char* name) override;

    void procUpdateServerGroups(std::vector<std::string>) override;
    void procAssignServerGroup(std::vector<std::string>) override;
    void procUnassignServerGroup(std::vector<std::string>) override;
    void procSendMessageToClient(std::vector<std::string>) override;

    DECLARE_MEMBER(STATE, State)
};

