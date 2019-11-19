#pragma once

#include "Common.h"
#include <signalrclient/signalr_value.h>


class ClientMessage {
public:
	ClientMessage(CLIENT_MESSAGE_TYPE procedureType, std::map<std::string, signalr::value> args);
    ClientMessage() = default;
    ~ClientMessage() = default;

	CLIENT_MESSAGE_TYPE getType();
	signalr::value getParameter(std::string key);

private:
    CLIENT_MESSAGE_TYPE m_ProcedureType;
	std::map<std::string, signalr::value> m_Parameters;
};
