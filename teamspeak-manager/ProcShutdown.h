#pragma once
#include "IProcedureFunction.h"
#include "Engine.h"

PROCEDURE_FUNCTION(ProcShutdown) {
    Engine::getInstance()->getClient()->procShutdown();
}

DECLARE_MEMBER(char *, Name)
};
