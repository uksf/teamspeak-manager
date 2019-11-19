#pragma once

#include "../Common/ClientMessage.h"
#include <functional>

class ISignalrClient {
public:
	virtual ~ISignalrClient() = default;

	virtual void initialize(std::function<void()> connectedCallback, std::function<void(ClientMessage)> procedureCallback) = 0;
	virtual void shutdown() = 0;
	virtual void sendMessage(SERVER_MESSAGE_TYPE procedure, signalr::value value) = 0;

DECLARE_INTERFACE_MEMBER(bool, Connected)
};
