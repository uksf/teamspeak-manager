#include "Engine.h"
#include "PipeManager.h"
#include "Ping.h"
#include "AssignServerGroup.h"
#include "UnassignServerGroup.h"

void Engine::initialize(IClient *client, IServer * commandServer) {
    if (!g_Log) {
        g_Log = static_cast<Log *>(new Log("tsm.log"));
        LOG("* Logging engine initialized.");
    }

    this->setClient(client);
    //this->setCommandServer(commandServer);
    this->m_PipeManager = new PipeManager();
    this->m_ProcedureEngine = new ProcedureEngine();

    this->getProcedureEngine()->addProcedure(new ping());
    this->getProcedureEngine()->addProcedure(new AssignServerGroup());
    this->getProcedureEngine()->addProcedure(new UnassignServerGroup());
}

void Engine::start() {
    LOG("Engine starting up");
    if (this->getPipeManager()) {
        this->getPipeManager()->initialize();
    }
    this->setState(STATE_RUNNING);
    LOG("Engine startup complte");
}

void Engine::stop() {
    LOG("Engine shutting down");
    this->setState(STATE_STOPPING);
    if (this->getProcedureEngine()) {
        this->getProcedureEngine()->stopWorker();
    }
    if (this->getPipeManager()) {
        this->getPipeManager()->shutdown();
    }
    this->setState(STATE_STOPPED);
    LOG("Engine shutdown complete");
}
