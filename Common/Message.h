#pragma once

#include "Common.h"
#include "IMessage.h"

#define MESSAGE_BUFFERSIZE 4096
#define MESSAGE_MAX_PARAMETER_COUNT 1024


class Message final : public IMessage {
public:
    Message(char* value, size_t len);
    ~Message();

    void parse(char* value, size_t len);
    char* getProcedureName() override;
    BOOL isValid() const;
    std::string getParameter(unsigned int index) override;
    int getParameterAsInt(unsigned int index) override;
    unsigned int getParameterCount() override;

    static IMessage* createNewMessage(char* procedureName, ...);
    static IMessage* formatNewMessage(char* procedureName, char* format, ...);

    std::string getData() override {
        return this->m_DataPtr;
    }

    void setData(unsigned char* data) {
        this->parse(reinterpret_cast<char *>(data), strlen(reinterpret_cast<char*>(data)));
    }

private:
    std::string* m_Data{};
    std::string* m_RpcProcedureName;
    std::string* m_Parameters[MESSAGE_MAX_PARAMETER_COUNT]{};
    unsigned int m_ParameterCount;
    BOOL m_IsValid{};
    char* m_DataPtr;
};
