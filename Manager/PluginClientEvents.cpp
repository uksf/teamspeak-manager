#include "Plugin.h"
#include "Engine.h"
#include "Data.h"

extern TS3Functions ts3Functions;

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
	logTSMessage("TS: Client move event tid: %u", std::this_thread::get_id());
	Engine::getInstance()->addToFunctionQueue([clientID, newChannelID, visibility]() {
		Data::getInstance()->handleClient(clientID, newChannelID, visibility);
	});
}

void ts3plugin_onClientMoveMovedEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID moverID,
                                      const char* moverName, const char* moverUniqueIdentifier, const char* moveMessage) {
    logTSMessage("TS: Client was moved: %d", clientID);
	Engine::getInstance()->addToFunctionQueue([clientID, newChannelID]() {
		Data::getInstance()->onClientMoveMoved(clientID, newChannelID);
	});
}

void ts3plugin_onClientKickFromChannelEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                            const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
    logTSMessage("TS: Client was kicked from channel: %d", clientID);
	Engine::getInstance()->addToFunctionQueue([clientID, newChannelID]() {
		Data::getInstance()->onClientKickFromChannel(clientID, newChannelID);
	});
}

void ts3plugin_onClientKickFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                           const char* kickerName, const char* kickerUniqueIdentifier, const char* kickMessage) {
    logTSMessage("TS: Client was kicked from server: %d", clientID);
	Engine::getInstance()->addToFunctionQueue([clientID]() {
		Data::getInstance()->onClientKickFromServer(clientID);
	});
}

void ts3plugin_onClientBanFromServerEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, anyID kickerID,
                                          const char* kickerName, const char* kickerUniqueIdentifier, uint64 time, const char* kickMessage) {
    logTSMessage("TS: Client was banned from server: %d", clientID);
	Engine::getInstance()->addToFunctionQueue([clientID]() {
		Data::getInstance()->onClientBanFromServer(clientID);
	});
}

void ts3plugin_onClientMoveTimeoutEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* timeoutMessage) {
    logTSMessage("TS: Client timed out: %d", clientID);
	Engine::getInstance()->addToFunctionQueue([clientID]() {
		Data::getInstance()->onClientMoveTimeout(clientID);
	});
}

void ts3plugin_onClientDisplayNameChanged(uint64 serverConnectionHandlerID, anyID clientID, const char* displayName, const char* uniqueClientIdentifier) {
    logTSMessage("TS: Client name changed: %d", clientID);
	Engine::getInstance()->addToFunctionQueue([clientID]() {
		Data::getInstance()->onClientDisplayName(clientID);
	});
}
