#pragma once

#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"
#include "../Common/ClientMessage.h"

PROCEDURE_FUNCTION(CLIENT_MESSAGE_TYPE::MESSAGE, ProcSendMessageToClient) {
        const uint64 clientDBID = std::stoul(std::to_string(message.getParameter("clientDbId").as_double()));
        const std::string messageString = message.getParameter("message").as_string();

        Engine::getInstance()->getClient()->procSendMessageToClient(clientDBID, messageString);
    }
};
