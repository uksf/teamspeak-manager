#include "Engine.h"
#include "../Common/Message.h"

void checkClientServerGroups(uint64 serverConnectionHandlerID, const char* clientUniqueIdentity) {
    const auto value = Engine::getInstance()->getUIDMapValue(clientUniqueIdentity);
    if (value.invalid) {
        if (value.clientDBID != NULL_UINT) {
            ts3Functions.requestServerGroupsByClientID(serverConnectionHandlerID, value.clientDBID, nullptr);
        } else {
            Engine::getInstance()->addToCallbackQueue(clientUniqueIdentity, DBID_QUEUE_MODE_GROUPS);
            ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUniqueIdentity, nullptr);
        }
    } else {
        Engine::getInstance()->addToCallbackQueue(clientUniqueIdentity, DBID_QUEUE_MODE_GROUPS);
        ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUniqueIdentity, nullptr);
    }
}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                             anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok && invokerClientID != selfId) {
        logTSMessage("Client server group change not invoked by me: %d", clientID);
        checkClientServerGroups(serverConnectionHandlerID, clientUniqueIdentity);
    }
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                               anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok && invokerClientID != selfId) {
        logTSMessage("Client server group change not invoked by me: %d", clientID);
        checkClientServerGroups(serverConnectionHandlerID, clientUniqueIdentity);
    }
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
    Engine::getInstance()->getSignalrClient()->sendMessage(
        Message::formatNewMessage(const_cast<char*>("CheckClientServerGroup"), const_cast<char*>("%d|%d"), clientDatabaseID, serverGroupList));
}
