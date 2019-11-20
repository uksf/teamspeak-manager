#pragma once

#include "ManagerCommon.h"
#include "../Common/Singleton.h"

class ClientMessage;

class ProcedureEngine final : public Singleton<ProcedureEngine> {
public:
	ProcedureEngine() = default;
	~ProcedureEngine() = default;

	void initialize();

	void addProcedure(IProcedure* cmd);
	void removeProcedure(IProcedure* cmd);
	void removeProcedure(CLIENT_MESSAGE_TYPE cmd);

	void runProcedure(ClientMessage message);
private:
	std::map<CLIENT_MESSAGE_TYPE, IProcedure*> m_Procedures;

	IProcedure* findProcedure(CLIENT_MESSAGE_TYPE cmd);
};
