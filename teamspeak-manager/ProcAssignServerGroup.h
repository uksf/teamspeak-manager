#pragma once

#include "IProcedureFunction.h"
#include "Engine.h"
#include "TextMessage.h"

PROCEDURE_FUNCTION(ProcAssignServerGroup) {
        const std::vector<std::string> details = {
            vMessage->getParameter(0),
            vMessage->getParameter(1)
        };

        Engine::getInstance()->getClient()->procAssignServerGroup(details);
    }

DECLARE_MEMBER(char *, Name)
};
