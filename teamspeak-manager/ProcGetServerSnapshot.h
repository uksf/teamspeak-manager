#pragma once

#include "IProcedureFunction.h"
#include "Engine.h"

PROCEDURE_FUNCTION(ProcGetServerSnapshot) {
    Engine::getInstance()->getClient()->procGetServerSnapshot();
}

DECLARE_MEMBER(char *, Name)
};
