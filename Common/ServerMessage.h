#pragma once

#include "Common.h"
#include <signalrclient/signalr_value.h>


class ServerMessage {
public:
	ServerMessage(SERVER_MESSAGE_TYPE procedureType, std::map<std::string, signalr::value> args);
	~ServerMessage() = default;

	SERVER_MESSAGE_TYPE getType();
	signalr::value getParameter(std::string key);

private:
	SERVER_MESSAGE_TYPE m_ProcedureType;
	std::map<std::string, signalr::value> m_Parameters;
};