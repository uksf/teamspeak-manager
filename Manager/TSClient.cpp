#include "TSClient.h"
#include "Engine.h"

extern TS3Functions ts3Functions;

int TSClient::checkIfBlacklisted(char* name) {
    if (strncmp(name, "serveradmin", 40) == 0) return 1;
    if (strncmp(name, "wukoIwZ1SPRWqrVlxyxmZypjpME=", 40) == 0) return 1;
    return 0;
}

void TSClient::procUpdateServerGroups(const uint64 clientDBID) {
    logTSMessage("Request server groups for %llu", clientDBID);
    if (ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to request server groups for %llu", clientDBID);
    }
}

void TSClient::procAssignServerGroup(const uint64 clientDBID, const uint64 serverGroupID) {
    logTSMessage("Assign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupAddClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to assign server group %llu for %llu", serverGroupID, clientDBID);
    }
}

void TSClient::procUnassignServerGroup(const uint64 clientDBID, const uint64 serverGroupID) {
    logTSMessage("Unassign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupDelClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to unassign server group %llu for %llu", serverGroupID, clientDBID);
    }
}

void TSClient::procSendMessageToClient(const uint64 clientDBID, const std::string message) {
    logTSMessage("Send message to %llu: '%s'", clientDBID, message.c_str());

    const MAP_DBID_VALUE clientUID = Engine::getInstance()->getDBIDMapValue(clientDBID);
    if (!clientUID.empty()) {
        const auto mapUIDValue = Engine::getInstance()->getUIDMapValue(clientUID);
        if (!mapUIDValue.invalid) {
            ts3Functions.requestSendPrivateTextMsg(ts3Functions.getCurrentServerConnectionHandlerID(), message.c_str(), mapUIDValue.clientID, nullptr);
        } else {
            logTSMessage("Failed to get ID from UID %s", clientUID.c_str());
        }
    } else {
        logTSMessage("Client UID not found in DBID map %llu, assuming offline", clientDBID);
    }
}
