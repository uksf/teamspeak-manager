#include <ts3_functions.h>
#include <teamspeak/public_errors.h>
#include "Plugin.h"
#include "Engine.h"

extern TS3Functions ts3Functions;

void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
    // update channel name
    logTSMessage("Channel edited: %llu", channelID);
    char* channelName;
    if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, channelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
        logTSMessage("Failed getting channel name");
        return;
    }
    Engine::getInstance()->updateUIDMapChannelName(channelID, channelName);
}
