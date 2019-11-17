#pragma once

#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"
#include "../Common/Message.h"

PROCEDURE_FUNCTION(ProcAssignServerGroup) {
        const std::vector<std::string> details = {
            vMessage->getParameter(0),
            vMessage->getParameter(1)
        };

        Engine::getInstance()->getClient()->procAssignServerGroup(details);
    }

DECLARE_MEMBER(char *, Name)
};
