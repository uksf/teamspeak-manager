#pragma once

#include "IProcedure.h"
#include "Procedures.h"

PROCEDURE_FUNCTION(CLIENT_MESSAGE_TYPE::RELOAD, ProcReload) {
		Procedures::getInstance()->reload();
    }
};
