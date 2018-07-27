#include "Plugin.h"
#include "Engine.h"
#include "TSClient.h"
#include "IServer.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "TextMessage.h"

#ifdef _WIN32
#define STRCPY(dest, destSize, src) strcpy_s(dest, destSize, src)
#define SNPRINTF sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 22

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
    return "1.0.0";
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
    Engine::getInstance()->initialize(new TSClient());

    return 0;
}

void ts3plugin_shutdown() {
    if (Engine::getInstance()->getClient()->getState() != STATE_STOPPED && Engine::getInstance()->getClient()->getState() != STATE_STOPPING) {
        Engine::getInstance()->getClient()->stop();
    }
    Engine::getInstance()->stop();
}

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
    if (visibility == ENTER_VISIBILITY) {
        DEBUG("Client connected: %d", clientID);
        char* clientUID;
        if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
            ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID, nullptr);
        }
    }
}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok) {
        if (invokerClientID != selfId) {
            DEBUG("Client server group change not invoked by me: %d", clientID);
            char* clientUID;
            if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
                ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID, nullptr);
            }
        }
    }
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID, anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok) {
        if (invokerClientID != selfId) {
            DEBUG("Client server group change not invoked by me: %d", clientID);
            char* clientUID;
            if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
                ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID, nullptr);
            }
        }
    }
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
    DEBUG("Client DBID retrieved: %d", clientDatabaseID);
    ts3Functions.requestServerGroupsByClientID(serverConnectionHandlerID, clientDatabaseID, nullptr);
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
    Engine::getInstance()->getPipeManager()->sendMessage(TextMessage::formatNewMessage("CheckClientServerGroup", "%d,%d", clientDatabaseID, serverGroupList));
}

