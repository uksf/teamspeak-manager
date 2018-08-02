#include "Plugin.h"
#include "Engine.h"
#include "TSClient.h"
#include "IServer.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"

extern TS3Functions ts3Functions;

std::string getClientUID(uint64 serverConnectionHandlerID, anyID clientID) {
    char* clientUID;
    std::string clientUIDString = Engine::getInstance()->getIDMapValue(clientID);
    if (clientUIDString.empty()) {
        if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) != ERROR_ok) {
            logTSMessage("Failed to get client UID: %d. Won't continue", clientID);
            return "";
        }
        if (Engine::getInstance()->getClient()->checkIfBlacklisted(clientUID)) return "";
        logTSMessage("Client UID not found in ID map, setting %d as %s", clientID, clientUID);
        Engine::getInstance()->updateOrSetIDMapValue(clientID, clientUID);
    } else {
        logTSMessage("Client UID found in ID map, getting %d as %s", clientID, clientUIDString.c_str());
        return clientUIDString;
    }
    return clientUID;
}

void updateClientChannel(uint64 serverConnectionHandlerID, std::string clientUID, uint64 newChannelID) {
    char* channelName;
    if (ts3Functions.getChannelVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), newChannelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
        logTSMessage("Failed getting channel name");
        return;
    }
    Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, NULL_ANYID, "", newChannelID, channelName);
}

// UID map
// key: UID
// value: DBID, clientID, clientName, channelID, channelName

// DBID map
// key: DBID
// value: UID

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
    // connected
    // store clientID, UID, clientName, channelID, channelName
    // add UID to DBID event queue in server group check mode
    // request DBID

    // move
    // update channelID and channelName
    // check we have DBID

    // disconnected
    // unset clientID and channelID

    std::string clientUID = getClientUID(serverConnectionHandlerID, clientID);
    if (clientUID.empty()) return;

    switch (visibility) {
    case ENTER_VISIBILITY: {
        logTSMessage("Client joined: %d", clientID);
        char* clientName;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
            logTSMessage("Failed getting client name");
            return;
        }
        Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, clientID, clientName, newChannelID, "");
        updateClientChannel(serverConnectionHandlerID, clientUID, newChannelID);
        Engine::getInstance()->addToCallbackQueue(clientUID, DBID_QUEUE_MODE_GROUPS);
        ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID.c_str(), nullptr);
        break;
    }
    case RETAIN_VISIBILITY: {
        logTSMessage("Client moved: %d", clientID);
        char* clientName;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
            logTSMessage("Failed getting client name");
            return;
        }
        Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, clientID, clientName, newChannelID, "");
        updateClientChannel(serverConnectionHandlerID, clientUID, newChannelID);
        const auto value = Engine::getInstance()->getUIDMapValue(clientUID);
        if (value.clientDBID == NULL_UINT) {
            logTSMessage("No DBID stored in UID map for: %s", clientUID.c_str());
            ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID.c_str(), nullptr);
        } else {
            logTSMessage("DBID found in UID map for: %s", clientUID.c_str());
        }
        break;
    }
    case LEAVE_VISIBILITY: {
        logTSMessage("Client left: %d, unsetting uid %s", clientID, clientUID.c_str());
        Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
        break;
    }
    default:
        logTSMessage("Visibility error", clientID);
    }
}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID,
                                      const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
    // update channelID and channelName
    logTSMessage("Client was moved: %d", clientID);
    std::string clientUID = getClientUID(serverConnectionHandlerID, clientID);
    if (clientUID.empty()) return;
    updateClientChannel(serverConnectionHandlerID, clientUID, newChannelID);
}

void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                            const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
    // update channelID and channelName
    logTSMessage("Client was kicked from channel: %d", clientID);
    std::string clientUID = getClientUID(serverConnectionHandlerID, clientID);
    if (clientUID.empty()) return;
    updateClientChannel(serverConnectionHandlerID, clientUID, newChannelID);
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                           const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
    // unset clientID and channelID, delete from ID map
    logTSMessage("Client was kicked from server: %d", clientID);
    std::string clientUID = getClientUID(serverConnectionHandlerID, clientID);
    if (clientUID.empty()) return;
    Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
    Engine::getInstance()->deleteIDMapValue(clientID);
}

void ts3plugin_onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                          const char* kickerName, const char* kickerUniqueIdentifier, uint64 time, const char* kickMessage) {
    // delete from all maps
    logTSMessage("Client was banned from server: %d", clientID);
    std::string clientUID = getClientUID(serverConnectionHandlerID, clientID);
    if (clientUID.empty()) return;
    const auto value = Engine::getInstance()->getUIDMapValue(clientUID);
    if (value.clientDBID != NULL_UINT) {
        Engine::getInstance()->deleteDBIDMapValue(value.clientDBID);
    }
    Engine::getInstance()->deleteUIDMapValue(clientUID);
    Engine::getInstance()->deleteIDMapValue(clientID);
}

void ts3plugin_onClientDisplayNameChanged(uint64 serverConnectionHandlerID, anyID clientID, const char* displayName, const char* uniqueClientIdentifier) {
    // update clientName
    logTSMessage("Client name changed: %d", clientID);
    std::string clientUID = getClientUID(serverConnectionHandlerID, clientID);
    if (clientUID.empty()) return;
    char* clientName;
    if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
        logTSMessage("Failed getting client name");
        return;
    }
    Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, NULL_ANYID, clientName, NULL_UINT, "");
}
