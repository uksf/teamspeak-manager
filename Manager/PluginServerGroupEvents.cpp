#include "Plugin.h"
#include "Engine.h"
#include "../Common/ClientMessage.h"

extern TS3Functions ts3Functions;

void checkClientServerGroups(const char* clientUniqueIdentity) {
    const auto value = Engine::getInstance()->getUIDMapValue(clientUniqueIdentity);
    if (!value.invalid) {
        if (value.clientDBID != NULL_UINT) {
            ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), value.clientDBID, nullptr);
        } else {
            Engine::getInstance()->addToCallbackQueue(clientUniqueIdentity, DBID_QUEUE_MODE::GROUPS);
            ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUniqueIdentity, nullptr);
        }
    } else {
        Engine::getInstance()->addToCallbackQueue(clientUniqueIdentity, DBID_QUEUE_MODE::GROUPS);
        ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUniqueIdentity, nullptr);
    }
}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                             anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok && invokerClientID != selfId) {
        logTSMessage("Client server group change not invoked by me: %d", clientID);
        checkClientServerGroups(clientUniqueIdentity);
    }
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                               anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok && invokerClientID != selfId) {
        logTSMessage("Client server group change not invoked by me: %d", clientID);
        checkClientServerGroups(clientUniqueIdentity);
    }
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
    const std::map<std::string, signalr::value> map{{"clientDbid", static_cast<double>(clientDatabaseID)}, {"serverGroupId", static_cast<double>(serverGroupList)}};
    Engine::getInstance()->getSignalrClient()->sendMessage(SERVER_MESSAGE_TYPE::CLIENT_SERVER_GROUPS, signalr::value(map));
}
