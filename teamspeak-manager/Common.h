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
#include <utility>
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

// UID map
// key: UID
// value: DBID, clientID, clientName, channelID, channelName
struct MAP_UID_VALUE {
    uint64 clientDBID{};
    anyID clientID{};
    std::string clientName;
    uint64 channelID{};
    std::string channelName;
    bool invalid = false;

    MAP_UID_VALUE() { invalid = true; }
    MAP_UID_VALUE(uint64 newDBID, anyID newClientID, std::string newClientName, uint64 newChannelID, std::string newChannelName) : clientDBID(newDBID), clientID(newClientID),
                                                                                                                                        clientName(std::move(newClientName)),
                                                                                                                                        channelID(newChannelID),
                                                                                                                                        channelName(std::move(newChannelName)) { }
};

typedef std::string MAP_UID_KEY;
typedef std::map<MAP_UID_KEY, MAP_UID_VALUE> MAP_UID;

// DBID map
// key: DBID
// value: UID
typedef uint64 MAP_DBID_KEY;
typedef std::string MAP_DBID_VALUE;
typedef std::map<MAP_DBID_KEY, MAP_DBID_VALUE> MAP_DBID;

// ID map
// key: ID
// value: UID
typedef anyID MAP_ID_KEY;
typedef std::string MAP_ID_VALUE;
typedef std::map<MAP_ID_KEY, MAP_ID_VALUE> MAP_ID;

// DBID callback queue
// pair: UID, QUEUE_MODE
typedef int DBID_QUEUE_MODE;
typedef std::map<MAP_UID_KEY, DBID_QUEUE_MODE> DBID_QUEUE;

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

void logTSMessage(char const* format, ...);
