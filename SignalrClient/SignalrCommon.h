#pragma once

#include "../Common/Common.h"
#include "../Common/Macros.h"

class IMessage;
class IProcedureFunction;

typedef struct PROCEDURE_DATA {
	IProcedureFunction* function;
	IMessage* message;

	PROCEDURE_DATA(IProcedureFunction* func, IMessage* msg) {
		function = func;
		message = msg;
	}

	PROCEDURE_DATA() {
		function = nullptr;
		message = nullptr;
	}
} PROCEDURE_DATA, * PPROCEDURE_DATA;