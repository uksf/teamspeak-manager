#pragma once

#include "../SignalrClient/IProcedureFunction.h"
#include "Engine.h"
#include "../Common/Message.h"

PROCEDURE_FUNCTION(ProcSendMessageToClient) {
    const std::vector<std::string> details = {
        vMessage->getParameter(0),
        vMessage->getParameter(1)
    };

    Engine::getInstance()->getClient()->procSendMessageToClient(details);
}

DECLARE_MEMBER(char *, Name)
};
