#pragma once

// ReSharper disable CppUnusedIncludeDirective
#define _WINSOCKAPI_
#define NOMINMAX

#ifdef _WIN32
#pragma warning (disable : 4100)
#include <Windows.h>
#endif

#include "Macros.h"
#include "Singleton.h"
#include <string>
#include <vector>
#include <ts3_functions.h>
#include <map>
#include <optional>
// ReSharper restore CppUnusedIncludeDirective

class IMessage;
class IServer;
class IProcedureFunction;

typedef unsigned int ID;
typedef unsigned int STATE;
typedef unsigned int CLIENTUID_MODE;
typedef std::optional<std::pair<anyID, CLIENTUID_MODE>> CLIENTUID_MODE_PAIR;
typedef std::map<std::string, CLIENTUID_MODE_PAIR> CLIENTUID_MODE_MAP;
typedef std::map<uint64, std::string> CLIENTDBID_MESSAGE_MAP;
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
