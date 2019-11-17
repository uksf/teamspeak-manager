#pragma once

#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"
#include "../Common/Message.h"

PROCEDURE_FUNCTION(ProcUnassignServerGroup) {
    const std::vector<std::string> details = {
        vMessage->getParameter(0),
        vMessage->getParameter(1)
    };

    Engine::getInstance()->getClient()->procUnassignServerGroup(details);
}

DECLARE_MEMBER(char *, Name)
};
