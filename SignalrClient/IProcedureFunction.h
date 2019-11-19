#pragma once

#include "SignalrCommon.h"

class IProcedureFunction {
public:
	virtual ~IProcedureFunction() = default;

	virtual void call(ClientMessage message) = 0;
	DECLARE_INTERFACE_MEMBER(CLIENT_MESSAGE_TYPE, Type)
};
