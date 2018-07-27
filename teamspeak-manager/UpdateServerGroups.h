#pragma once

#include "IProcedureFunction.h"
#include "Engine.h"
#include "TextMessage.h"

PROCEDURE_FUNCTION(UpdateServerGroups) {
    const std::vector<std::string> details = {
        std::string(reinterpret_cast<char *>(vMessage->getParameter(0))),
    };

    Engine::getInstance()->getClient()->updateServerGroups(details);
}

DECLARE_MEMBER(char *, Name)
};
