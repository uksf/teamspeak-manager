#include "Plugin.h"
#include "Engine.h"
#include "TSClient.h"
#include "IServer.h"
#include "teamspeak/public_definitions.h"

#ifdef _WIN32
#define STRCPY(dest, destSize, src) strcpy_s(dest, destSize, src)
#define SNPRINTF sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 23

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

TS3Functions ts3Functions;

const char* ts3plugin_name() {
    return "Teamspeak Manager";
}

const char* ts3plugin_version() {
    return "1.1.0";
}

int ts3plugin_apiVersion() {
    return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
    return "Tim Beswick";
}

const char* ts3plugin_description() {
    return "Manages teamspeak interactions";
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

int ts3plugin_init() {
    logTSMessage("TSM Loaded");
    Engine::getInstance()->initialize(new TSClient());
    if (ts3Functions.getCurrentServerConnectionHandlerID()) {
        ts3plugin_onConnectStatusChangeEvent(ts3Functions.getCurrentServerConnectionHandlerID(), STATUS_CONNECTION_ESTABLISHED, NULL);
    }

    return 0;
}

void ts3plugin_onConnectStatusChangeEvent(uint64 id, int status, unsigned int err) {
    if (status == STATUS_CONNECTION_ESTABLISHED) {
        ts3Functions.requestChannelSubscribeAll(ts3Functions.getCurrentServerConnectionHandlerID(), nullptr);
        if (Engine::getInstance()->getClient()->getState() != STATE_RUNNING) {
            Engine::getInstance()->getClient()->start();
        }
    } else if (status == STATUS_DISCONNECTED) {
        if (Engine::getInstance()->getClient()->getState() != STATE_STOPPED && Engine::getInstance()->getClient()->getState() != STATE_STOPPING) {
            Engine::getInstance()->getClient()->stop();
        }
    }
}

void ts3plugin_shutdown() {
    if (Engine::getInstance()->getClient()->getState() != STATE_STOPPED && Engine::getInstance()->getClient()->getState() != STATE_STOPPING) {
        Engine::getInstance()->getClient()->stop();
    }
    Engine::getInstance()->stop();
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
    logTSMessage("Client DBID retrieved: %llu", clientDatabaseID);

    // add dbid to dbid map, and set in uid map
    const std::string clientUID(uniqueClientIdentifier);
    Engine::getInstance()->updateOrSetDBIDMapValue(clientDatabaseID, clientUID);
    Engine::getInstance()->updateOrSetUIDMapValue(clientUID, clientDatabaseID, NULL_ANYID, "", NULL_UINT, "");

    // pop queue and handle
    const DBID_QUEUE_MODE callback = Engine::getInstance()->getFromCallbackQueue(clientUID);
    if (callback == DBID_QUEUE_MODE_UNSET) {
        logTSMessage("No callback to handle in DBID queue for UID %s", clientUID.c_str());
        return;
    }
    const auto mapUIDValue = Engine::getInstance()->getUIDMapValue(clientUID);
    switch (callback) {
    case DBID_QUEUE_MODE_GROUPS:
        logTSMessage("Found Groups callback to handle");
        ts3Functions.requestServerGroupsByClientID(serverConnectionHandlerID, mapUIDValue.clientDBID, nullptr);
        return;
    default: break;
    }
    logTSMessage("Invalid callback mode");
}

void logTSMessage(char const* format, ...) {
    char message[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof message, format, args);
    ts3Functions.logMessage(message, LogLevel_INFO, "TSM", ts3Functions.getCurrentServerConnectionHandlerID());
    va_end(args);
}
