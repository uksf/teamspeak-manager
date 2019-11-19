#pragma once
#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"

PROCEDURE_FUNCTION(CLIENT_MESSAGE_TYPE::SHUTDOWN, ProcShutdown) {
        Engine::getInstance()->procShutdown();
    }
};
