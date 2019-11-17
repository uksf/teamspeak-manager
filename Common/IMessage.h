#pragma once
#include <string>

class IMessage {
public:
    virtual ~IMessage() = default;

    virtual std::string getData(void) = 0;
    virtual char* getProcedureName(void) = 0;

    virtual std::string getParameter(unsigned int) = 0;
    virtual int getParameterAsInt(unsigned int) = 0;
    virtual unsigned int getParameterCount(void) = 0;

};
