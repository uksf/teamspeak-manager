#pragma once

#include "IProcedureFunction.h"
#include "Engine.h"
#include "TextMessage.h"

PROCEDURE_FUNCTION(ProcUpdateServerGroups) {
    const std::vector<std::string> details = {
        vMessage->getParameter(0),
    };

    Engine::getInstance()->getClient()->procUpdateServerGroups(details);
}

DECLARE_MEMBER(char *, Name)
};
