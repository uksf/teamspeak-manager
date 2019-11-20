#pragma once

#include "IProcedure.h"
#include "../Common/ClientMessage.h"

PROCEDURE_FUNCTION(CLIENT_MESSAGE_TYPE::ASSIGN, ProcAssignServerGroup) {
        const uint64 clientDBID = std::stoul(std::to_string(message.getParameter("clientDbId").as_double()));
        const uint64 serverGroup = std::stoul(std::to_string(message.getParameter("serverGroup").as_double()));

		Procedures::getInstance()->assignServerGroup(clientDBID, serverGroup);
    }
};
