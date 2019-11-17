#pragma once

#include "../Common/Message.h"
#include <functional>

class ISignalrClient {
public:
	virtual ~ISignalrClient() = default;

	virtual void initialize(std::function<void()> connectedCallback, std::function<void(IMessage*)> procedureCallback) = 0;
	virtual void shutdown() = 0;
	virtual void sendMessage(IMessage* msg) = 0;

	DECLARE_INTERFACE_MEMBER(BOOL, Connected)
};
