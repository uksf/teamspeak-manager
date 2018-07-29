#pragma once

#include "IProcedureFunction.h"
#include "Engine.h"
#include "TextMessage.h"

PROCEDURE_FUNCTION(ProcUnassignServerGroup) {
    const std::vector<std::string> details = {
        std::string(reinterpret_cast<char *>(vMessage->getParameter(0))),
        std::string(reinterpret_cast<char *>(vMessage->getParameter(1)))
    };

    Engine::getInstance()->getClient()->procUnassignServerGroup(details);
}

DECLARE_MEMBER(char *, Name)
};
