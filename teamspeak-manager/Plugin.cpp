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
    return "1.0.1";
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
    ts3Functions.logMessage("TSM Loaded", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
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
        char* clientUID;
        if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
            if (Engine::getInstance()->getClient()->checkIfBlacklisted(clientUID)) return;
            char msg[1024];
            snprintf(msg, sizeof msg, "Client connected: %d", clientID);
            ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
            Engine::getInstance()->setClientUIDMode(clientUID, CLIENTUID_MODE_PAIR{std::pair<anyID, CLIENTUID_MODE>{clientID, CLIENTUID_MODE_GROUPS}});
            ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID, nullptr);
        }
    }
}

void ts3plugin_onServerGroupClientAddedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                             anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok) {
        if (invokerClientID != selfId) {
            char* clientUID;
            if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
                if (Engine::getInstance()->getClient()->checkIfBlacklisted(clientUID)) return;
                char msg[1024];
                snprintf(msg, sizeof msg, "Client server group change not invoked by me: %d", clientID);
                ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
                Engine::getInstance()->setClientUIDMode(clientUID, CLIENTUID_MODE_PAIR{std::pair<anyID, CLIENTUID_MODE>{clientID, CLIENTUID_MODE_GROUPS}});
                ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID, nullptr);
            }
        }
    }
}

void ts3plugin_onServerGroupClientDeletedEvent(uint64 serverConnectionHandlerID, anyID clientID, const char* clientName, const char* clientUniqueIdentity, uint64 serverGroupID,
                                               anyID invokerClientID, const char* invokerName, const char* invokerUniqueIdentity) {
    anyID selfId;
    if (ts3Functions.getClientID(serverConnectionHandlerID, &selfId) == ERROR_ok) {
        if (invokerClientID != selfId) {
            char* clientUID;
            if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
                if (Engine::getInstance()->getClient()->checkIfBlacklisted(clientUID)) return;
                char msg[1024];
                snprintf(msg, sizeof msg, "Client server group change not invoked by me: %d", clientID);
                ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
                Engine::getInstance()->setClientUIDMode(clientUID, CLIENTUID_MODE_PAIR{std::pair<anyID, CLIENTUID_MODE>{clientID, CLIENTUID_MODE_GROUPS}});
                ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID, nullptr);
            }
        }
    }
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
    char msg[1024];
    snprintf(msg, sizeof msg, "Client DBID retrieved: %llu", clientDatabaseID);
    ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);

    auto pair = Engine::getInstance()->getClientUIDMode(uniqueClientIdentifier);
    if (!pair.has_value()) {
        ts3Functions.logMessage("Client UID Mode pair doesn't have a value", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
        return;
    }
    if (pair.value().second != CLIENTUID_MODE_UNSET) {
        switch (pair.value().second) {
        case CLIENTUID_MODE_GROUPS:
            ts3Functions.requestServerGroupsByClientID(serverConnectionHandlerID, clientDatabaseID, nullptr);
            return;
        case CLIENTUID_MODE_SNAPSHOT:
            Engine::getInstance()->getClient()->finishSnapshotForClient(pair.value().first, clientDatabaseID);
            return;
        case CLIENTUID_MODE_ONLINE:
            Engine::getInstance()->getClient()->finishOnlineForClient(pair.value().first, clientDatabaseID);
            return;
        default: break;
        }
    }
    ts3Functions.logMessage("Client UID not found in UID mode map, won't continue", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
}

void ts3plugin_onServerGroupByClientIDEvent(uint64 serverConnectionHandlerID, const char* name, uint64 serverGroupList, uint64 clientDatabaseID) {
    Engine::getInstance()->getPipeManager()->sendMessage(TextMessage::formatNewMessage("CheckClientServerGroup", "%d|%d", clientDatabaseID, serverGroupList));
}

void ts3plugin_onClientNamefromDBIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID, const char* clientNickName) {
    char msg[1024];
    snprintf(msg, sizeof msg, "Client name from DBID retrieved: %llu, %s", clientDatabaseID, clientNickName);
    ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);

    auto message = Engine::getInstance()->getClientDBIDMessage(clientDatabaseID);
    if (message.empty()) {
        ts3Functions.logMessage("Client DBID message doesn't have a value", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
    }

    anyID* clientList;
    if (ts3Functions.getClientList(serverConnectionHandlerID, &clientList) != ERROR_ok) {
        return;
    }
    while (*clientList) {
        const anyID clientID = *clientList;
        clientList++;
        char* clientUID;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
            if (strncmp(uniqueClientIdentifier, clientUID, 100) == 0) {
                if (ts3Functions.requestSendPrivateTextMsg(serverConnectionHandlerID, message.c_str(), clientID, nullptr) != ERROR_ok) {
                    char emsg[1024];
                    snprintf(emsg, sizeof emsg, "Failed to send message to %s: '%s'", clientNickName, message.c_str());
                    ts3Functions.logMessage(emsg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
                }
                return;
            }
        }
    }
}
