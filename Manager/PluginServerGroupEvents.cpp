#include "Plugin.h"
#include "Engine.h"
#include "../Common/ClientMessage.h"
#include "Data.h"

extern TS3Functions ts3Functions;

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                             anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok && invokerClientID != selfId) {
        logTSMessage("TS: Client server group change not invoked by me: %d", clientID);
		Engine::getInstance()->addToFunctionQueue([clientUniqueIdentity]() {
			Data::getInstance()->checkClientServerGroups(clientUniqueIdentity);
		});
    }
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                               anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok && invokerClientID != selfId) {
        logTSMessage("TS: Client server group change not invoked by me: %d", clientID);
		Engine::getInstance()->addToFunctionQueue([clientUniqueIdentity]() {
			Data::getInstance()->checkClientServerGroups(clientUniqueIdentity);
		});
    }
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
	logTSMessage("TS: Server group event tid: %u", std::this_thread::get_id());
    const std::map<std::string, signalr::value> map{{"clientDbid", static_cast<double>(clientDatabaseID)}, {"serverGroupId", static_cast<double>(serverGroupList)}};
	Engine::getInstance()->addToSendQueue(SERVER_MESSAGE_TYPE::CLIENT_SERVER_GROUPS, signalr::value(map));
}
