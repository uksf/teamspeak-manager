#pragma once

#include "Common.h"

class IClient {
public:
    virtual void stop() = 0;
    virtual void start() = 0;

    virtual void updateServerGroups(std::vector<std::string>) = 0;
    virtual void assignServerGroup(std::vector<std::string>) = 0;
    virtual void unassignServerGroup(std::vector<std::string>) = 0;

    DECLARE_INTERFACE_MEMBER(STATE, State)
};
