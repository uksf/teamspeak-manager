#pragma once

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32)
#pragma warning (disable : 4100)
#include <Windows.h>
#endif

#include "Macros.h"

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <optional>

enum class STATE {
	STOPPED,
	RUNNING,
	STOPPING
};

constexpr auto NULL_ANYID = short(0);
constexpr auto UNSET_ANYID = MAXSHORT;
constexpr auto NULL_UINT = 0llu;

enum class DBID_QUEUE_MODE {
	UNSET,
	SNAPSHOT,
	GROUPS,
	ONLINE
};

enum class SERVER_MESSAGE_TYPE {
	EMPTY_EVENT,
	CLIENTS,
	CLIENT_SERVER_GROUPS	
};

enum class CLIENT_MESSAGE_TYPE {
	EMPTY_PROCEDURE,
	ASSIGN,
	UNASSIGN,
	GROUPS,
	MESSAGE,
	SHUTDOWN	
};

void logTSMessage(char const* format, ...);
