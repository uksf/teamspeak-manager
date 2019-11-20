#pragma once

#include "ManagerCommon.h"
#include "../Common/Singleton.h"

class Procedures : public Singleton<Procedures> {
public:
    Procedures() = default;
    ~Procedures() = default;

    static void updateServerGroups(uint64 clientDBID);
    static void assignServerGroup(uint64 clientDBID, uint64 serverGroupID);
    static void unassignServerGroup(uint64 clientDBID, uint64 serverGroupID);
    static void sendMessageToClient(uint64 clientDBID, std::string message);
    static void shutdown();
};

