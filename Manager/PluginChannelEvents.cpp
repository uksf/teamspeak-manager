#include <ts3_functions.h>
#include <teamspeak/public_errors.h>
#include "Plugin.h"
#include "Engine.h"
#include "Data.h"

extern TS3Functions ts3Functions;

void ts3plugin_onUpdateChannelEditedEvent(uint64 serverConnectionHandlerID, uint64 channelID, anyID invokerID, const char* invokerName, const char* invokerUniqueIdentifier) {
    // update channel name
    logTSMessage("TS: Channel edited: %llu", channelID);
    char* channelName;
    if (ts3Functions.getChannelVariableAsString(serverConnectionHandlerID, channelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
        logTSMessage("TS: Failed getting channel name");
        return;
    }

	Engine::getInstance()->addToFunctionQueue([channelID, channelName]() {
		Data::getInstance()->updateUIDMapChannelName(channelID, channelName);
	});
}
