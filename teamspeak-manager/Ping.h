#pragma once
#include "IProcedureFunction.h"
#include "IServer.h"
#include "TextMessage.h"

volatile DWORD g_pingTime;

PROCEDURE_FUNCTION(ping) {
        vServer->sendMessage(TextMessage::formatNewMessage("Pong", ""));
    }

DECLARE_MEMBER(char *, Name)
};
