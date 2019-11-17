#pragma once

#include "SignalrCommon.h"

class IProcedureFunction {
public:
	virtual ~IProcedureFunction() = default;

	virtual void call(IMessage*) = 0;
	DECLARE_INTERFACE_MEMBER(char*, Name)
};
