#include "Engine.h"
#include "PipeManager.h"
#include "Ping.h"
#include "AssignServerGroup.h"
#include "UnassignServerGroup.h"
#include "UpdateServerGroups.h"

extern TS3Functions ts3Functions;

void Engine::initialize(IClient *client) {
    this->setClient(client);
    this->m_PipeManager = new PipeManager();
    this->m_ProcedureEngine = new ProcedureEngine();

    this->getProcedureEngine()->addProcedure(new ping());
    this->getProcedureEngine()->addProcedure(new UpdateServerGroups());
    this->getProcedureEngine()->addProcedure(new AssignServerGroup());
    this->getProcedureEngine()->addProcedure(new UnassignServerGroup());

    this->getClient()->start();
}

void Engine::start() {
    ts3Functions.logMessage("Engine starting up", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
    if (this->getPipeManager()) {
        this->getPipeManager()->initialize();
    }
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
    this->setState(STATE_STOPPED);
    ts3Functions.logMessage("Engine shutdown complete", LogLevel_INFO, "Plugin", ts3Functions.getCurrentServerConnectionHandlerID());
}
