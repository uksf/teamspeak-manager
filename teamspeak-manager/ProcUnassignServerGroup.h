#pragma once

#include "IProcedureFunction.h"
#include "Engine.h"
#include "TextMessage.h"

PROCEDURE_FUNCTION(ProcUnassignServerGroup) {
    const std::vector<std::string> details = {
        vMessage->getParameter(0),
        vMessage->getParameter(1)
    };

    Engine::getInstance()->getClient()->procUnassignServerGroup(details);
}

DECLARE_MEMBER(char *, Name)
};
