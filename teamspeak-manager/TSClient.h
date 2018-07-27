#pragma once

#include "IClient.h"

class TSClient : public IClient {
public:
    TSClient() = default;
    ~TSClient() = default;

    void stop() override;
    void start() override;

    void updateServerGroups(std::vector<std::string>) override;
    void assignServerGroup(std::vector<std::string>) override;
    void unassignServerGroup(std::vector<std::string>) override;

    DECLARE_MEMBER(STATE, State)
};

