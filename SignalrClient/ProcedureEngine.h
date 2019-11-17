#pragma once

#include "SignalrCommon.h"
#include "../Common/Worker.h"
#include <map>
#include <string>

class ProcedureEngine final : public Worker<PROCEDURE_DATA*> {
public:
	ProcedureEngine();
	~ProcedureEngine();

	void addProcedure(IProcedureFunction* cmd);
	void removeProcedure(IProcedureFunction* cmd);
	void removeProcedure(char* cmd);
	IProcedureFunction* findProcedure(char* cmd);
	void runProcedure(IMessage* msg);
	void exProcessItem(PROCEDURE_DATA* data) override;
private:
	std::map<std::string, IProcedureFunction*> m_FunctionList;
};
