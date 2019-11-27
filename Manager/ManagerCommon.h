#pragma once

#include <teamspeak/public_errors.h>
#include <teamspeak/public_errors_rare.h>
#include <teamspeak/public_definitions.h>
#include <teamspeak/public_rare_definitions.h>
#include <teamspeak/clientlib_publicdefinitions.h>
#include <ts3_functions.h>
#include "../Common/Common.h"
#include "../Common/ClientMessage.h"
#include "IProcedure.h"

// UID map
// key: UID
// value: DBID, clientID, clientName, channelID, channelName
struct MAP_UID_VALUE {
	uint64 clientDBID{};
	anyID clientID{};
	std::string clientName;
	uint64 channelID{};
	std::string channelName;

	MAP_UID_VALUE() = default;
	MAP_UID_VALUE(const uint64 newDBID, const anyID newClientID, std::string newClientName, const uint64 newChannelID, std::string newChannelName) : clientDBID(newDBID),
		clientID(newClientID),
		clientName(std::move(
		newClientName)),
		channelID(newChannelID),
		channelName(std::move(
		newChannelName)) { }
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
typedef DBID_QUEUE_MODE DBID_QUEUE_MODE;
typedef std::map<MAP_UID_KEY, DBID_QUEUE_MODE> DBID_QUEUE;

typedef std::pair<IProcedure*, ClientMessage> ProcedurePair;
