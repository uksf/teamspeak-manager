#include "SignalrCommon.h"
#include "ProcedureEngine.h"
#include "IProcedureFunction.h"
#include "../Common/ClientMessage.h"

ProcedureEngine::ProcedureEngine() {
    this->startWorker();
}

ProcedureEngine::~ProcedureEngine() {
    this->stopWorker();
}

void ProcedureEngine::exProcessItem(ProcedurePair procedure) {
    if (procedure.first) {
		procedure.first->call(procedure.second);
    }
}

void ProcedureEngine::addProcedure(IProcedureFunction* cmd) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_FunctionList.insert(std::pair<CLIENT_MESSAGE_TYPE, IProcedureFunction*>(cmd->getType(), cmd));
    }
}

void ProcedureEngine::removeProcedure(IProcedureFunction* cmd) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_FunctionList.erase(cmd->getType());
    }
}

void ProcedureEngine::removeProcedure(CLIENT_MESSAGE_TYPE cmd) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_FunctionList.erase(cmd);
    }
}

IProcedureFunction* ProcedureEngine::findProcedure(CLIENT_MESSAGE_TYPE cmd) {
    if (this->getShuttingDown()) {
        return nullptr;
    }
    const auto it = this->m_FunctionList.find(cmd);
    if (it != this->m_FunctionList.end()) {
        return static_cast<IProcedureFunction*>(it->second);
    }
    return nullptr;
}

void ProcedureEngine::runProcedure(ClientMessage message) {
    if (message.getType() == CLIENT_MESSAGE_TYPE::EMPTY_PROCEDURE) {
        return;
    }

    IProcedureFunction* function = this->findProcedure(message.getType());
    if (function) {
        if (!this->getRunning()) {
            this->startWorker();
        }
        {
            std::lock_guard<std::mutex> lock(m_lockable_mutex);
            this->m_processQueue.push(ProcedurePair(function, message));
        }
    }
}
