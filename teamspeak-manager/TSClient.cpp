#include "TSClient.h"
#include "Engine.h"
#include <teamspeak/public_errors.h>
#include "IServer.h"
#include "TextMessage.h"

extern TS3Functions ts3Functions;

#define INVALID_TS3_CHANNEL -1

void TSClient::start() {
    Engine::getInstance()->start();
    this->setState(STATE_RUNNING);
}

void TSClient::stop() {
    if (Engine::getInstance() != nullptr) {
        Engine::getInstance()->stop();
        this->setState(STATE_STOPPING);
        this->m_LastSnapshotClient = 0;
        this->setState(STATE_STOPPED);
    }
}

int TSClient::checkIfBlacklisted(char* name) {
    if (strncmp(name, "serveradmin", 100) == 0) return 1;
    if (strncmp(name, "b2o0zBVHLeHG5gjULDfxxyHD8C0=", 100) == 0) return 1;
    return 0;
}

void TSClient::procUpdateServerGroups(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));

    char logmsg[1024];
    snprintf(logmsg, sizeof logmsg, "Request server groups for %llu", clientDBID);
    ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    if (ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), clientDBID, nullptr) != ERROR_ok) {
        char logmsge[1024];
        snprintf(logmsge, sizeof logmsge, "Failed to request server groups for %llu", clientDBID);
        ts3Functions.logMessage(logmsge, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

void TSClient::procAssignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    char logmsg[1024];
    snprintf(logmsg, sizeof logmsg, "Assign server group %llu for %llu", serverGroupID, clientDBID);
    ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    if (ts3Functions.requestServerGroupAddClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        char logmsge[1024];
        snprintf(logmsge, sizeof logmsge, "Failed to assign server group %llu for %llu", serverGroupID, clientDBID);
        ts3Functions.logMessage(logmsge, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

void TSClient::procUnassignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    char logmsg[1024];
    snprintf(logmsg, sizeof logmsg, "Unassign server group %llu for %llu", serverGroupID, clientDBID);
    ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    if (ts3Functions.requestServerGroupDelClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        char logmsge[1024];
        snprintf(logmsge, sizeof logmsge, "Failed to unassign server group %llu for %llu", serverGroupID, clientDBID);
        ts3Functions.logMessage(logmsge, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

void TSClient::procGetServerSnapshot() {
    ts3Functions.logMessage("Getting server snapshot", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    this->m_LastSnapshotClient = 0;

    anyID* clients;
    if (ts3Functions.getClientList(ts3Functions.getCurrentServerConnectionHandlerID(), &clients) != ERROR_ok) {
        ts3Functions.logMessage("Failed getting client list", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
        return;
    }
    while (*clients) {
        anyID clientID = *clients;
        clients++;
        char m1[1024];
        snprintf(m1, sizeof m1, "Requesting dbid for %hu for snapshot", clientID);
        ts3Functions.logMessage(m1, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
        if (!*clients) {
            ts3Functions.logMessage("No more clients, setting this as the last one", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
            this->m_LastSnapshotClient = clientID;
        }
        char* clientUID;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) == ERROR_ok) {
            if (checkIfBlacklisted(clientUID)) continue;
            Engine::getInstance()->setClientUIDMode(clientUID, CLIENTUID_MODE_PAIR{std::pair<anyID, CLIENTUID_MODE>{clientID, CLIENTUID_MODE_SNAPSHOT}});
            char m3[1024];
            snprintf(m3, sizeof m3, "Requesting dbid for %hu", clientID);
            ts3Functions.logMessage(m3, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
            if (ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUID, nullptr) != ERROR_ok) {
                ts3Functions.logMessage("Failed requesting client dbid from uid", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
                continue;
            }
            ts3Functions.freeMemory(clientUID);
        }
    }
    char m2[1024];
    snprintf(m2, sizeof m2, "Snapshot request complete, last client is %hu", this->m_LastSnapshotClient);
    ts3Functions.logMessage(m2, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
}

void TSClient::finishSnapshotForClient(const anyID clientID, const uint64 clientDatabaseID) {
    uint64 channelId;
    if (ts3Functions.getChannelOfClient(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, &channelId) != ERROR_ok) {
        ts3Functions.logMessage("Failed getting client channel", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
        return;
    }
    char* channelName;
    if (ts3Functions.getChannelVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), channelId, CHANNEL_NAME, &channelName) != ERROR_ok) {
        ts3Functions.logMessage("Failed getting channel name", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
        return;
    }
    char* clientName;
    if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
        ts3Functions.logMessage("Failed getting client name", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
        return;
    }
    char m1[1024];
    snprintf(m1, sizeof m1, "Sending snapshot for client %hu. Last client is %hu", clientID, this->m_LastSnapshotClient);
    ts3Functions.logMessage(m1, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    const int lastClient = this->m_LastSnapshotClient > 0 && this->m_LastSnapshotClient == clientID;
    if (lastClient) {
        ts3Functions.logMessage("This is the last client", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
        this->m_LastSnapshotClient = 0;
    }
    Engine::getInstance()->getPipeManager()->sendMessage(
        TextMessage::formatNewMessage("StoreServerSnapshot", "%d,%s,%d,%s,%d", clientDatabaseID, clientName, channelId, channelName, lastClient));
}

void TSClient::procSendMessageToClient(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const std::string message = args.at(1);

    char logmsg[1024];
    snprintf(logmsg, sizeof logmsg, "Send message to %llu: '%s'", clientDBID, message.c_str());
    ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    Engine::getInstance()->setClientDBIDMessage(clientDBID, message);
    if (ts3Functions.requestClientNamefromDBID(ts3Functions.getCurrentServerConnectionHandlerID(), clientDBID, nullptr) != ERROR_ok) {
        char logmsge[1024];
        snprintf(logmsge, sizeof logmsge, "Failed to get client name from DBID %llu", clientDBID);
        ts3Functions.logMessage(logmsge, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

void TSClient::procShutdown() {
    this->stop();
    ts3Functions.logMessage("Disconnecting", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    ts3Functions.stopConnection(ts3Functions.getCurrentServerConnectionHandlerID(), "Jarvis needs to reboot");
    Sleep(2000);
    TerminateProcess(GetCurrentProcess(), 0);
}
