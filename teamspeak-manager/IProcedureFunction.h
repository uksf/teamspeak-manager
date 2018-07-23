#pragma once

#include "Common.h"

class IProcedureFunction
{
public:
    virtual ~IProcedureFunction() = default;

    virtual void call(IServer*, IMessage *) = 0;
    DECLARE_INTERFACE_MEMBER(char *, Name)
};