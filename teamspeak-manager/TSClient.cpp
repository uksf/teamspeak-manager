#include "TSClient.h"
#include "Engine.h"
#include <teamspeak/public_errors.h>
#include "Plugin.h"

extern TS3Functions ts3Functions;

#define INVALID_TS3_CHANNEL (-1)

void TSClient::start() {
    Engine::getInstance()->start();
    this->setState(STATE_RUNNING);
}

void TSClient::stop() {
    if (Engine::getInstance() != nullptr) {
        Engine::getInstance()->stop();
        this->setState(STATE_STOPPING);
        this->setState(STATE_STOPPED);
    }
}

int TSClient::checkIfBlacklisted(char* name) {
    if (strncmp(name, "serveradmin", 40) == 0) return 1;
    if (strncmp(name, "b2o0zBVHLeHG5gjULDfxxyHD8C0=", 40) == 0) return 1;
    return 0;
}

void TSClient::procInitaliseClientMaps() {
    Engine::getInstance()->initaliseClientMaps();
}

void TSClient::procUpdateServerGroups(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));

    logTSMessage("Request server groups for %llu", clientDBID);
    if (ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to request server groups for %llu", clientDBID);
    }
}

void TSClient::procAssignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    logTSMessage("Assign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupAddClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Failed to assign server group %llu for %llu", serverGroupID, clientDBID);
    }
}

void TSClient::procUnassignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    logTSMessage("Unassign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupDelClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
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
            ts3Functions.requestSendPrivateTextMsg(ts3Functions.getCurrentServerConnectionHandlerID(), message.c_str(), mapUIDValue.clientID, nullptr);
        } else {
            logTSMessage("Failed to get ID from UID %s", clientUID.c_str());
        }
    } else {
        logTSMessage("Client UID not found in DBID map %llu, assuming offline", clientDBID);
    }
}

void TSClient::procShutdown() {
    logTSMessage("Disconnecting");
    ts3Functions.stopConnection(ts3Functions.getCurrentServerConnectionHandlerID(), "Jarvis needs to reboot");
    Sleep(2000);
    TerminateProcess(GetCurrentProcess(), 0);
}
