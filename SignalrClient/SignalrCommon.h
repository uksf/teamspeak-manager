#pragma once

#include "../Common/ClientMessage.h"

class IProcedureFunction;

//typedef struct PROCEDURE_DATA {
//	IProcedureFunction* function;
//	ClientMessage message;
//
//	PROCEDURE_DATA(IProcedureFunction* func, const ClientMessage msg) {
//		function = func;
//		message = msg;
//	}
//
//	PROCEDURE_DATA() {
//        function = nullptr;
//    }
//} PROCEDURE_DATA, * PPROCEDURE_DATA;

typedef std::pair<IProcedureFunction*, ClientMessage> ProcedurePair;