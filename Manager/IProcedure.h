#pragma once

#include "ManagerCommon.h"

class IProcedure {
public:
    virtual ~IProcedure() = default;

    virtual void call(ClientMessage message) = 0;
public:
    virtual void setType(CLIENT_MESSAGE_TYPE value) = 0;
    virtual CLIENT_MESSAGE_TYPE getType() = 0;
};
