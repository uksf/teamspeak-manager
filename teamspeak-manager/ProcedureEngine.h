#pragma once

#include "Common.h"
#include "EntrantWorker.h"
#include <map>
#include <string>

class ProcedureEngine : public EntrantWorker<PROCEDURE_DATA *>
{
public:
    ProcedureEngine();
    ~ProcedureEngine();

    void addProcedure(IProcedureFunction *cmd);
    void removeProcedure(IProcedureFunction *cmd);
    void removeProcedure(char * cmd);
    IProcedureFunction *findProcedure(char *cmd);
    void runProcedure(IServer *serverInstance, IMessage *msg);
    void runProcedure(IServer *serverInstance, IMessage *msg, BOOL entrant);
    void exProcessItem(PROCEDURE_DATA *data) override;
private:
    std::map<std::string, IProcedureFunction *> m_FunctionList;
};
