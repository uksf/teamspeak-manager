#include "ClientMessage.h"
#include <string>

ClientMessage::ClientMessage(CLIENT_MESSAGE_TYPE procedureType, std::map<std::string, signalr::value> args) {
	this->m_ProcedureType = procedureType;
	this->m_Parameters = args;
}

signalr::value ClientMessage::getParameter(std::string key) {
    if (this->m_Parameters.contains(key)) {
		return this->m_Parameters[key];
    }
    return "";
}

CLIENT_MESSAGE_TYPE ClientMessage::getType() {
    return this->m_ProcedureType;
}
