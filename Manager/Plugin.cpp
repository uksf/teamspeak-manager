#include "Plugin.h"
#include "ManagerCommon.h"
#include "Engine.h"
#include "Data.h"

constexpr auto PLUGIN_API_VERSION = 26;

TS3Functions ts3Functions;

const char* ts3plugin_name() {
	return "Teamspeak Manager";
}

const char* ts3plugin_version() {
	return "2.0.0";
}

int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

const char* ts3plugin_author() {
	return "Tim Beswick";
}

const char* ts3plugin_description() {
	return "Manages interactions with UKSF API";
}

int ts3plugin_requestAutoload() {
	return 1;
}

void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
	ts3Functions = funcs;
}

int ts3plugin_init() {
	logTSMessage("Loaded");
	logTSMessage("TS: Main tid: %u", std::this_thread::get_id());
	Engine::getInstance()->initialize();
	if (ts3Functions.getCurrentServerConnectionHandlerID()) {
		ts3plugin_onConnectStatusChangeEvent(ts3Functions.getCurrentServerConnectionHandlerID(), STATUS_CONNECTION_ESTABLISHED, NULL);
	}

	return 0;
}

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int status, unsigned int err) {
	logTSMessage("TS: Status change tid: %u", std::this_thread::get_id());
	if (status == STATUS_CONNECTION_ESTABLISHED) {
		ts3Functions.requestChannelSubscribeAll(serverConnectionHandlerID, nullptr);
		if (Engine::getInstance()->getState() != STATE::RUNNING) {
			Engine::getInstance()->start();
		}
	} else if (status == STATUS_DISCONNECTED) {
		if (Engine::getInstance()->getState() != STATE::STOPPED && Engine::getInstance()->getState() != STATE::STOPPING) {
			Engine::getInstance()->stop();
		}
	}
}

void ts3plugin_shutdown() {
	if (Engine::getInstance()->getState() != STATE::STOPPED && Engine::getInstance()->getState() != STATE::STOPPING) {
		Engine::getInstance()->stop();
	}
}

void ts3plugin_onChannelSubscribeFinishedEvent(uint64 serverConnectionHandlerID) {
	logTSMessage("TS: Channel subscribe finish event tid: %u", std::this_thread::get_id());
	Engine::getInstance()->addToFunctionQueue([]() {
		Data::getInstance()->populateClientMaps();
	});
}

void ts3plugin_onClientDBIDfromUIDEvent(uint64 serverConnectionHandlerID, const char* uniqueClientIdentifier, uint64 clientDatabaseID) {
	logTSMessage("TS: Client DBID %llu for %s event tid: %u", clientDatabaseID, uniqueClientIdentifier, std::this_thread::get_id());
	auto clientUID = std::string(uniqueClientIdentifier);
	Engine::getInstance()->addToFunctionQueue([clientUID, clientDatabaseID]() {
		Data::getInstance()->onClientDBIDfromUID(clientUID, clientDatabaseID);
	});
}

void logTSMessage(char const* format, ...) {
	char message[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(message, sizeof message, format, args);
	ts3Functions.logMessage(message, LogLevel_INFO, "UKSF Manager", ts3Functions.getCurrentServerConnectionHandlerID());
	va_end(args);
}
