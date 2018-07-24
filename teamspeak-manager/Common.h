#pragma once

// ReSharper disable CppUnusedIncludeDirective
#define _WINSOCKAPI_
#define NOMINMAX
#undef UNICODE

#ifdef _WIN32
#pragma warning (disable : 4100)
#include <Windows.h>
#endif

#include "Macros.h"
#include "Singleton.h"
#include <string>
#include <vector>
#include <ts3_functions.h>
// ReSharper restore CppUnusedIncludeDirective

class IMessage;
class IServer;
class IProcedureFunction;

static struct TS3Functions ts3Functions;
typedef unsigned int STATE;
typedef unsigned int ID;
typedef struct PROCEDURE_DATA {
    IProcedureFunction* function;
    IServer* server;
    IMessage* message;

    PROCEDURE_DATA(IProcedureFunction* func, IServer* serv, IMessage* msg) {
        function = func;
        server = serv;
        message = msg;
    }

    PROCEDURE_DATA() {
        function = nullptr;
        server = nullptr;
        message = nullptr;
    }
} PROCEDURE_DATA, *PPROCEDURE_DATA;
