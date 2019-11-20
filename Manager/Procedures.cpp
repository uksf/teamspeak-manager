#include "Procedures.h"
#include "Engine.h"
#include "Data.h"

extern TS3Functions ts3Functions;

void Procedures::updateServerGroups(const uint64 clientDBID) {
    logTSMessage("Procedure: Request server groups for %llu", clientDBID);
    if (ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Procedure: Failed to request server groups for %llu", clientDBID);
    }
}

void Procedures::assignServerGroup(const uint64 clientDBID, const uint64 serverGroupID) {
    logTSMessage("Procedure: Assign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupAddClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Procedure: Failed to assign server group %llu for %llu", serverGroupID, clientDBID);
    }
}

void Procedures::unassignServerGroup(const uint64 clientDBID, const uint64 serverGroupID) {
    logTSMessage("Procedure: Unassign server group %llu for %llu", serverGroupID, clientDBID);
    if (ts3Functions.requestServerGroupDelClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        logTSMessage("Procedure: Failed to unassign server group %llu for %llu", serverGroupID, clientDBID);
    }
}

void Procedures::sendMessageToClient(const uint64 clientDBID, const std::string message) {
    logTSMessage("Procedure: Send message to %llu: '%s'", clientDBID, message.c_str());

    const MAP_DBID_VALUE clientUID = Data::getInstance()->getDBIDMapValue(clientDBID);
    if (!clientUID.empty()) {
        const auto mapUIDValue = Data::getInstance()->getUIDMapValue(clientUID);
        if (!mapUIDValue.invalid) {
            ts3Functions.requestSendPrivateTextMsg(ts3Functions.getCurrentServerConnectionHandlerID(), message.c_str(), mapUIDValue.clientID, nullptr);
        } else {
            logTSMessage("Procedure: Failed to get ID from UID %s", clientUID.c_str());
        }
    } else {
        logTSMessage("Procedure: Client UID not found in DBID map %llu, assuming offline", clientDBID);
    }
}

void Procedures::shutdown() {
	logTSMessage("Procedure: Stopping via API");
	Engine::getInstance()->stop();
	ts3Functions.stopConnection(ts3Functions.getCurrentServerConnectionHandlerID(), "Jarvis needs to reboot");
	Sleep(2000);
	TerminateProcess(GetCurrentProcess(), 0);
}
