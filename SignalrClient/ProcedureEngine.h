#pragma once

#include "SignalrCommon.h"
#include "../Common/Worker.h"
#include <map>

class ClientMessage;

class ProcedureEngine final : public Worker<ProcedurePair> {
public:
	ProcedureEngine();
	~ProcedureEngine();

	void addProcedure(IProcedureFunction* cmd);
	void removeProcedure(IProcedureFunction* cmd);
	void removeProcedure(CLIENT_MESSAGE_TYPE cmd);
	IProcedureFunction* findProcedure(CLIENT_MESSAGE_TYPE cmd);
	void runProcedure(ClientMessage message);
	void exProcessItem(ProcedurePair procedure) override;
private:
	std::map<CLIENT_MESSAGE_TYPE, IProcedureFunction*> m_FunctionList;
};
