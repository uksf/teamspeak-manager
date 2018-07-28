#include "TSClient.h"
#include "Engine.h"
#include <teamspeak/public_errors.h>

extern TS3Functions ts3Functions;

void TSClient::start() {
    Engine::getInstance()->start();
    this->setState(STATE_RUNNING);
}

void TSClient::stop() {
    if (Engine::getInstance() != nullptr) {
        Engine::getInstance()->stop();
        this->setState(STATE_STOPPING);
        this->setState(STATE_STOPPED);
    }
}

void TSClient::updateServerGroups(std::vector<std::string> args) {
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

void TSClient::assignServerGroup(std::vector<std::string> args) {
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

void TSClient::unassignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    char logmsg[1024];
    snprintf(logmsg, sizeof logmsg, "Unassign server group %llu for %llu", serverGroupID, clientDBID);
    ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    if (ts3Functions.requestServerGroupDelClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr) != ERROR_ok) {
        char logmsge[1024];
        snprintf(logmsg, sizeof logmsge, "Failed to unassign server group %llu for %llu", serverGroupID, clientDBID);
        ts3Functions.logMessage(logmsge, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

