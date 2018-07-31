#include "Engine.h"
#include "PipeManager.h"
#include "ProcPing.h"
#include "ProcAssignServerGroup.h"
#include "ProcUnassignServerGroup.h"
#include "ProcUpdateServerGroups.h"
#include "ProcGetServerSnapshot.h"
#include "ProcSendMessageToClient.h"
#include "ProcShutdown.h"
#include "ProcGetOnlineClients.h"

extern TS3Functions ts3Functions;

void Engine::initialize(IClient* client) {
    this->setClient(client);
    this->m_PipeManager = new PipeManager();
    this->m_ProcedureEngine = new ProcedureEngine();

    this->getProcedureEngine()->addProcedure(new ProcPing());
    this->getProcedureEngine()->addProcedure(new ProcShutdown());
    this->getProcedureEngine()->addProcedure(new ProcUpdateServerGroups());
    this->getProcedureEngine()->addProcedure(new ProcAssignServerGroup());
    this->getProcedureEngine()->addProcedure(new ProcUnassignServerGroup());
    this->getProcedureEngine()->addProcedure(new ProcGetServerSnapshot());
    this->getProcedureEngine()->addProcedure(new ProcSendMessageToClient());
    this->getProcedureEngine()->addProcedure(new ProcGetOnlineClients());

    this->getClient()->start();
}

void Engine::start() {
    ts3Functions.logMessage("Engine starting up", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    if (this->getPipeManager()) {
        this->getPipeManager()->initialize();
    }
    this->m_ClientUIDModeMap.clear();
    this->m_ClientDBIDMessageMap.clear();
    this->setState(STATE_RUNNING);
    ts3Functions.logMessage("Engine startup complete", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
}

void Engine::stop() {
    ts3Functions.logMessage("Engine shutting down", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    this->setState(STATE_STOPPING);
    if (this->getProcedureEngine()) {
        this->getProcedureEngine()->stopWorker();
    }
    if (this->getPipeManager()) {
        this->getPipeManager()->shutdown();
    }
    this->m_ClientUIDModeMap.clear();
    this->m_ClientDBIDMessageMap.clear();
    this->setState(STATE_STOPPED);
    ts3Functions.logMessage("Engine shutdown complete", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
}

void Engine::setClientUIDMode(std::string clientUID, CLIENTUID_MODE_PAIR pair) {
    if (this->m_ClientUIDModeMap.count(clientUID) == 0) {
        this->m_ClientUIDModeMap.emplace(clientUID, pair);
    } else {
        char logmsg[1024];
        snprintf(logmsg, sizeof logmsg, "ClientUID mode already set as %d for %s", pair.value().second, clientUID.c_str());
        ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

CLIENTUID_MODE_PAIR Engine::getClientUIDMode(const std::string clientUID) {
    if (this->m_ClientUIDModeMap.count(clientUID) == 1) {
        const auto it = this->m_ClientUIDModeMap.find(clientUID);
        CLIENTUID_MODE_PAIR pair = it->second.value();
        this->m_ClientUIDModeMap.erase(it);
        return pair;
    }
    char logmsg[1024];
    snprintf(logmsg, sizeof logmsg, "ClientUID mode not found for %s", clientUID.c_str());
    ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    return {};
}

void Engine::setClientDBIDMessage(uint64 clientUID, std::string message) {
    if (this->m_ClientDBIDMessageMap.count(clientUID) == 0) {
        this->m_ClientDBIDMessageMap.emplace(clientUID, message);
    } else {
        char logmsg[1024];
        snprintf(logmsg, sizeof logmsg, "ClientDBID message already set for %llu as %s", clientUID, message.c_str());
        ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    }
}

std::string Engine::getClientDBIDMessage(const uint64 clientUID) {
    if (this->m_ClientDBIDMessageMap.count(clientUID) == 1) {
        const auto it = this->m_ClientDBIDMessageMap.find(clientUID);
        std::string message = it->second;
        this->m_ClientDBIDMessageMap.erase(it);
        return message;
    }
    char logmsg[1024];
    snprintf(logmsg, sizeof logmsg, "ClientDBID mode not found for %llu", clientUID);
    ts3Functions.logMessage(logmsg, LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    return "";
}
