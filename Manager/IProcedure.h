#pragma once

#include "ManagerCommon.h"

class IProcedure {
public:
	virtual ~IProcedure() = default;

	virtual void call(ClientMessage message) = 0;
	DECLARE_INTERFACE_MEMBER(CLIENT_MESSAGE_TYPE, Type)
};
