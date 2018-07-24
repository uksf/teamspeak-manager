#include "Plugin.h"
#include "Engine.h"
#include "TSClient.h"
#include "IServer.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "TextMessage.h"
#include "CommandServer.h"

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

//char *pluginID = nullptr;
//void ts3plugin_registerPluginID(const char* commandID) {
//    pluginID = _strdup(commandID);
//    if (Engine::getInstance() != NULL) {
//        if (dynamic_cast<CommandServer *>(Engine::getInstance()->getCommandServer()) != nullptr) {
//            dynamic_cast<CommandServer *>(Engine::getInstance()->getCommandServer())->setCommandId(pluginID);
//        }
//    }
//}
//
//void ts3plugin_onPluginCommandEvent(uint64 serverConnectionHandlerID, const char* pluginName, const char* pluginCommand) {
//    if (pluginName && pluginCommand) {
//        if (strstr(pluginName, "tsm") != nullptr && Engine::getInstance()->getCommandServer()) {
//            Engine::getInstance()->getCommandServer()->handleMessage((unsigned char *)pluginCommand);
//        }
//    }
//}

int ts3plugin_init() {
    Engine::getInstance()->initialize(new TSClient(), new CommandServer());
    //if (pluginID != nullptr) dynamic_cast<CommandServer *>(Engine::getInstance()->getCommandServer())->setCommandId(pluginID);
    if (ts3Functions.getCurrentServerConnectionHandlerID()) {
        ts3plugin_onConnectStatusChangeEvent(ts3Functions.getCurrentServerConnectionHandlerID(), STATUS_CONNECTION_ESTABLISHED, NULL);
    }

    return 0;
}

void ts3plugin_onConnectStatusChangeEvent(uint64 id, const int status, unsigned int err) {
    if (status == STATUS_CONNECTION_ESTABLISHED) {
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

void ts3plugin_onClientMoveEvent(uint64 serverConnectionHandlerID, anyID clientID, uint64 oldChannelID, uint64 newChannelID, int visibility, const char* moveMessage) {
    if (visibility == ENTER_VISIBILITY) {
        LOG("Client connected: %d", clientID);

        char* clientUID;
        if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
            ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID, nullptr);
        }
    }
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
    LOG("Client DBID retrieved: %d", clientDatabaseID);
    Engine::getInstance()->getPipeManager()->sendMessage(TextMessage::formatNewMessage("CheckClientServerGroups", "%d", clientDatabaseID));
}
