#include "Plugin.h"
#include "Engine.h"

extern TS3Functions ts3Functions;

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
    Engine::getInstance()->handleClient(clientID, newChannelID, visibility);
}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID,
                                      const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
    // update channelID and channelName
    logTSMessage("Client was moved: %d", clientID);
    const std::string clientUID = Engine::getInstance()->getClientUID(clientID);
    if (clientUID.empty()) return;
    Engine::getInstance()->updateClientChannel(clientUID, newChannelID);
}

void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                            const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
    // update channelID and channelName
    logTSMessage("Client was kicked from channel: %d", clientID);
    const std::string clientUID = Engine::getInstance()->getClientUID(clientID);
    if (clientUID.empty()) return;
    Engine::getInstance()->updateClientChannel(clientUID, newChannelID);
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                           const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
    // unset clientID and channelID, delete from ID map
    logTSMessage("Client was kicked from server: %d", clientID);
    const std::string clientUID = Engine::getInstance()->getClientUID(clientID);
    if (clientUID.empty()) return;
    Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
    Engine::getInstance()->deleteIDMapValue(clientID);
}

void ts3plugin_onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                          const char* kickerName, const char* kickerUniqueIdentifier, uint64 time, const char* kickMessage) {
    // delete from all maps
    logTSMessage("Client was banned from server: %d", clientID);
    const std::string clientUID = Engine::getInstance()->getClientUID(clientID);
    if (clientUID.empty()) return;
    const auto value = Engine::getInstance()->getUIDMapValue(clientUID);
    if (value.clientDBID != NULL_UINT) {
        Engine::getInstance()->deleteDBIDMapValue(value.clientDBID);
    }
    Engine::getInstance()->deleteUIDMapValue(clientUID);
    Engine::getInstance()->deleteIDMapValue(clientID);
}

void ts3plugin_onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* timeoutMessage) {
    // unset clientID and channelID, delete from ID map
    logTSMessage("Client timed out: %d", clientID);
    const std::string clientUID = Engine::getInstance()->getClientUID(clientID);
    if (clientUID.empty()) return;
    Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
    Engine::getInstance()->deleteIDMapValue(clientID);
}

void ts3plugin_onClientDisplayNameChanged(uint64 serverConnectionHandlerID, anyID clientID, const char* displayName, const char* uniqueClientIdentifier) {
    // update clientName
    logTSMessage("Client name changed: %d", clientID);
    const std::string clientUID = Engine::getInstance()->getClientUID(clientID);
    if (clientUID.empty()) return;
    char* clientName;
    if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
        logTSMessage("Failed getting client name");
        return;
    }
    Engine::getInstance()->updateOrSetUIDMapValue(clientUID, NULL_UINT, NULL_ANYID, clientName, NULL_UINT, "");
}
