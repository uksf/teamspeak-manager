#pragma once
#include "IProcedureFunction.h"
#include "IServer.h"
#include "TextMessage.h"
#include <time.h>

volatile DWORD g_pingTime;

PROCEDURE_FUNCTION(ping) {
        g_pingTime = clock() / CLOCKS_PER_SEC;
        vServer->sendMessage(TextMessage::formatNewMessage("Pong", "%f,", g_pingTime));
    }

DECLARE_MEMBER(char *, Name)
};
