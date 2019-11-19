#pragma once

#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"

PROCEDURE_FUNCTION(CLIENT_MESSAGE_TYPE::GROUPS, ProcUpdateServerGroups) {
        const uint64 clientDBID = std::stoul(std::to_string(message.getParameter("clientDbId").as_double()));

        Engine::getInstance()->getClient()->procUpdateServerGroups(clientDBID);
    }
};
