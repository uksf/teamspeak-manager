#pragma once

#include "IProcedureFunction.h"
#include "Engine.h"
#include "TextMessage.h"

PROCEDURE_FUNCTION(AssignServerGroup) {
        const std::vector<std::string> details = {
            std::string(reinterpret_cast<char *>(vMessage->getParameter(0))),
            std::string(reinterpret_cast<char *>(vMessage->getParameter(1)))
        };

        Engine::getInstance()->getClient()->assignServerGroup(details);
    }

DECLARE_MEMBER(char *, Name)
};
