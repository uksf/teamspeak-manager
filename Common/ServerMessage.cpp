#include "ServerMessage.h"
#include <string>

ServerMessage::ServerMessage(SERVER_MESSAGE_TYPE procedureType, std::map<std::string, signalr::value> args) {
	this->m_ProcedureType = procedureType;
	this->m_Parameters = args;
}

signalr::value ServerMessage::getParameter(std::string key) {
	if (this->m_Parameters.contains(key)) {
		return this->m_Parameters[key];
	}
	return "";
}

SERVER_MESSAGE_TYPE ServerMessage::getType() {
	return this->m_ProcedureType;
}
