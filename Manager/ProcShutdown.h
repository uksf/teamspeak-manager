#pragma once
#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"

PROCEDURE_FUNCTION(ProcShutdown) {
    Engine::getInstance()->procShutdown();
}

DECLARE_MEMBER(char *, Name)
};
