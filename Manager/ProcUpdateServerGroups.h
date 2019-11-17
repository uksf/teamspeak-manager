#pragma once

#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"
#include "../Common/Message.h"

PROCEDURE_FUNCTION(ProcUpdateServerGroups) {
    const std::vector<std::string> details = {
        vMessage->getParameter(0),
    };

    Engine::getInstance()->getClient()->procUpdateServerGroups(details);
}

DECLARE_MEMBER(char *, Name)
};
