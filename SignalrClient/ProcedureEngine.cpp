#include "SignalrCommon.h"
#include "ProcedureEngine.h"
#include "IProcedureFunction.h"
#include "../Common/IMessage.h"

ProcedureEngine::ProcedureEngine() {
	this->startWorker();
}

ProcedureEngine::~ProcedureEngine() {
	this->stopWorker();
}

void ProcedureEngine::exProcessItem(PROCEDURE_DATA* data) {
	if (data->function) {
		data->function->call(data->message);
	}
	delete data->message;
	free(data);
}

void ProcedureEngine::addProcedure(IProcedureFunction* cmd) {
	{
		std::lock_guard<std::mutex> lock(m_lockable_mutex);
		this->m_FunctionList.insert(std::pair<std::string, IProcedureFunction*>(std::string(cmd->getName()), cmd));
	}
}

void ProcedureEngine::removeProcedure(IProcedureFunction* cmd) {
	{
		std::lock_guard<std::mutex> lock(m_lockable_mutex);
		this->m_FunctionList.erase(cmd->getName());
	}
}

void ProcedureEngine::removeProcedure(char* cmd) {
	{
		std::lock_guard<std::mutex> lock(m_lockable_mutex);
		this->m_FunctionList.erase(cmd);
	}
}

IProcedureFunction* ProcedureEngine::findProcedure(char* cmd) {
	if (this->getShuttingDown()) {
		return nullptr;
	}
	const auto it = this->m_FunctionList.find(std::string(static_cast<char*>(cmd)));
	if (it != this->m_FunctionList.end()) {
		return static_cast<IProcedureFunction*>(it->second);
	}
	return nullptr;
}

void ProcedureEngine::runProcedure(IMessage* msg) {
	if (msg) {
		if (!msg->getProcedureName()) {
			delete msg;
			return;
		}

		IProcedureFunction* ptr = this->findProcedure(msg->getProcedureName());
		if (ptr) {
			if (!this->getRunning()) {
				this->startWorker();
			}
			auto* data = static_cast<PROCEDURE_DATA*>(malloc(sizeof(PROCEDURE_DATA)));
			data->function = ptr;
			data->message = msg;
			{
				std::lock_guard<std::mutex> lock(m_lockable_mutex);
				this->m_processQueue.push(data);
			}
		} else {
			delete msg;
		}
	}
}
