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
        
        // Fix: Convert const char* to std::string to avoid dangling pointer
        std::string clientUID(clientUniqueIdentity ? clientUniqueIdentity : "");
        logTSMessage("TS: Server group added - capturing UID: '%s' for clientID: %d", clientUID.c_str(), clientID);
        
		Engine::getInstance()->addToFunctionQueue([clientUID]() {
            logTSMessage("TS: Server group added - executing with captured UID: '%s'", clientUID.c_str());
			Data::getInstance()->checkClientServerGroups(clientUID.c_str());
		});
    }
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                               anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok && invokerClientID != selfId) {
        logTSMessage("TS: Client server group change not invoked by me: %d", clientID);
        
        // Fix: Convert const char* to std::string to avoid dangling pointer
        std::string clientUID(clientUniqueIdentity ? clientUniqueIdentity : "");
        logTSMessage("TS: Server group deleted - capturing UID: '%s' for clientID: %d", clientUID.c_str(), clientID);
        
		Engine::getInstance()->addToFunctionQueue([clientUID]() {
            logTSMessage("TS: Server group deleted - executing with captured UID: '%s'", clientUID.c_str());
			Data::getInstance()->checkClientServerGroups(clientUID.c_str());
		});
    }
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
	logTSMessage("TS: Server group event tid: %u", std::this_thread::get_id());
    const std::map<std::string, signalr::value> map{{"clientDbid", static_cast<double>(clientDatabaseID)}, {"serverGroupId", static_cast<double>(serverGroupList)}};
	Engine::getInstance()->addToSendQueue(SERVER_MESSAGE_TYPE::CLIENT_SERVER_GROUPS, signalr::value(map));
}
