#include "Engine.h"
#include "PipeManager.h"
#include "Ping.h"
#include "AssignServerGroup.h"
#include "UnassignServerGroup.h"
#include "UpdateServerGroups.h"

void Engine::initialize(IClient *client) {
    if (!g_Log) {
        g_Log = static_cast<Log *>(new Log("tsm.log"));
        DEBUG("* Logging engine initialized.");
    }

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
    DEBUG("Engine starting up");
    if (this->getPipeManager()) {
        this->getPipeManager()->initialize();
    }
    this->setState(STATE_RUNNING);
    DEBUG("Engine startup complte");
}

void Engine::stop() {
    DEBUG("Engine shutting down");
    this->setState(STATE_STOPPING);
    if (this->getProcedureEngine()) {
        this->getProcedureEngine()->stopWorker();
    }
    if (this->getPipeManager()) {
        this->getPipeManager()->shutdown();
    }
    this->setState(STATE_STOPPED);
    DEBUG("Engine shutdown complete");
}
