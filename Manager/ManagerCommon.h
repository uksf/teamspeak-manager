#pragma once

#include "../Common/Common.h"
#include <teamspeak/public_errors.h>
#include <teamspeak/public_errors_rare.h>
#include <teamspeak/public_definitions.h>
#include <teamspeak/public_rare_definitions.h>
#include <teamspeak/clientlib_publicdefinitions.h>
#include <ts3_functions.h>
#include "Plugin.h"

static struct TS3Functions ts3Functions;

class IMessage;
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

    MAP_UID_VALUE() {
        invalid = true;
    }

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

static uint64 ServerConnectionHandlerID;