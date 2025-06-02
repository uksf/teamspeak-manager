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

    // Fix: Convert char* to std::string to avoid dangling pointer and add null check
    std::string channelNameStr(channelName ? channelName : "");
    logTSMessage("TS: Channel edited - capturing name: '%s' for channelID: %llu", channelNameStr.c_str(), channelID);
    
    // Free the memory allocated by TeamSpeak API
    if (channelName) {
        ts3Functions.freeMemory(channelName);
    }

	Engine::getInstance()->addToFunctionQueue([channelID, channelNameStr]() {
        logTSMessage("TS: Channel edited - executing with captured name: '%s' for channelID: %llu", channelNameStr.c_str(), channelID);
		Data::getInstance()->updateUIDMapChannelName(channelID, channelNameStr);
	});
}
