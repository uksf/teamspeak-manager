#pragma once
#include "IProcedureFunction.h"
#include "Engine.h"

PROCEDURE_FUNCTION(ProcGetOnlineClients) {
    Engine::getInstance()->getClient()->procGetOnlineClients();
}

DECLARE_MEMBER(char *, Name)
};
