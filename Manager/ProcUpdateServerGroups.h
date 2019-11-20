#pragma once

#include "IProcedure.h"
#include "Procedures.h"

PROCEDURE_FUNCTION(CLIENT_MESSAGE_TYPE::GROUPS, ProcUpdateServerGroups) {
        const uint64 clientDBID = std::stoul(std::to_string(message.getParameter("clientDbId").as_double()));

		Procedures::getInstance()->updateServerGroups(clientDBID);
    }
};
