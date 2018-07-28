#include "Common.h"
#include "ProcedureEngine.h"
#include "IProcedureFunction.h"
#include "IMessage.h"

ProcedureEngine::ProcedureEngine() {
    this->startWorker();
}

ProcedureEngine::~ProcedureEngine() {
    this->stopWorker();
}

void ProcedureEngine::exProcessItem(PROCEDURE_DATA* data) {
    if (data->function) {
        data->function->call(data->server, data->message);
    }
    delete data->message;
    free(data);
}

void ProcedureEngine::addProcedure(IProcedureFunction* cmd) {
    LOCK(this);
    this->m_FunctionList.insert(std::pair<std::string, IProcedureFunction *>(std::string(cmd->getName()), cmd));
    UNLOCK(this);
}

void ProcedureEngine::removeProcedure(IProcedureFunction* cmd) {
    LOCK(this);
    this->m_FunctionList.erase(cmd->getName());
    UNLOCK(this);
}

void ProcedureEngine::removeProcedure(char* cmd) {
    LOCK(this);
    this->m_FunctionList.erase(cmd);
    UNLOCK(this);
}

IProcedureFunction* ProcedureEngine::findProcedure(char* cmd) {
    if (this->getShuttingDown()) {
        return nullptr;
    }
    const auto it = this->m_FunctionList.find(std::string(static_cast<char *>(cmd)));
    if (it != this->m_FunctionList.end()) {
        return static_cast<IProcedureFunction *>(it->second);
    }
    return nullptr;
}

void ProcedureEngine::runProcedure(IServer* serverInstance, IMessage* msg) {
    return this->runProcedure(serverInstance, msg, 1);
}

void ProcedureEngine::runProcedure(IServer* serverInstance, IMessage* msg, const BOOL entrant) {
    if (msg) {
        if (!msg->getProcedureName()) {
            delete msg;
        }

        IProcedureFunction * ptr = this->findProcedure(msg->getProcedureName());
        if (ptr) {
            if (!entrant) {
                LOCK(this);
                ptr->call(serverInstance, msg);
                delete msg;
                UNLOCK(this);
            } else {
                if (!this->getRunning()) {
                    this->startWorker();
                }
                auto* data = static_cast<PROCEDURE_DATA *>(malloc(sizeof(PROCEDURE_DATA)));
                data->function = ptr;
                data->server = serverInstance;
                data->message = msg;
                LOCK(this);
                this->m_processQueue.push(data);
                UNLOCK(this);
            }
        } else {
            delete msg;
        }
    }
}
