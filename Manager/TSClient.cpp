#include "TSClient.h"
#include "Engine.h"

int TSClient::checkIfBlacklisted(char* name) {
    if (strncmp(name, "serveradmin", 40) == 0) return 1;
    if (strncmp(name, "wukoIwZ1SPRWqrVlxyxmZypjpME=", 40) == 0) return 1;
    return 0;
}

void TSClient::procUpdateServerGroups(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));

    logTSMessage("Request server groups for %llu", clientDBID);
    if (ts3Functions.requestServerGroupsByClientID(ServerConnectionHandlerID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to request server groups for %llu", clientDBID);
    }
}

void TSClient::procAssignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    logTSMessage("Assign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupAddClient(ServerConnectionHandlerID, serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to assign server group %llu for %llu", serverGroupID, clientDBID);
    }
}

void TSClient::procUnassignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    logTSMessage("Unassign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupDelClient(ServerConnectionHandlerID, serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to unassign server group %llu for %llu", serverGroupID, clientDBID);
    }
}

void TSClient::procSendMessageToClient(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoull(args.at(0));
    const std::string message = args.at(1);
    logTSMessage("Send message to %llu: '%s'", clientDBID, message.c_str());

    MAP_DBID_VALUE clientUID = Engine::getInstance()->getDBIDMapValue(clientDBID);
    if (!clientUID.empty()) {
        const auto mapUIDValue = Engine::getInstance()->getUIDMapValue(clientUID);
        if (!mapUIDValue.invalid) {
            ts3Functions.requestSendPrivateTextMsg(ServerConnectionHandlerID, message.c_str(), mapUIDValue.clientID, nullptr);
        } else {
            logTSMessage("Failed to get ID from UID %s", clientUID.c_str());
        }
    } else {
        logTSMessage("Client UID not found in DBID map %llu, assuming offline", clientDBID);
    }
}
