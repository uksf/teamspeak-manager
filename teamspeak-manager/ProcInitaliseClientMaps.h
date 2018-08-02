#pragma once
#include "IProcedureFunction.h"
#include "Engine.h"

PROCEDURE_FUNCTION(ProcInitaliseClientMaps) {
    Engine::getInstance()->getClient()->procInitaliseClientMaps();
}

DECLARE_MEMBER(char *, Name)
};
