#pragma once

class IMessage
{
public:
    virtual ~IMessage(){}

    virtual std::string getData(void) = 0;
    virtual unsigned int getLength(void) = 0;
    virtual char *getProcedureName(void) = 0;

    virtual std::string getParameter(unsigned int) = 0;
    virtual int getParameterAsInt(unsigned int) = 0;
    virtual unsigned int getParameterCount(void) = 0;
    
};