#pragma once

#include "IProcedure.h"
#include "Procedures.h"

PROCEDURE_FUNCTION(CLIENT_MESSAGE_TYPE::SHUTDOWN, ProcShutdown) {
        Procedures::getInstance()->shutdown();
    }
};
