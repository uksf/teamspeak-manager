#pragma once

#include "Common.h"
#include "IMessage.h"

#define TEXTMESSAGE_BUFSIZE 4096
#define TEXTMESSAGE_MAX_PARAMETER_COUNT 1024


class TextMessage : public IMessage {
public:
    TextMessage(char* value, size_t len);
    ~TextMessage();

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
    std::string* m_Parameters[TEXTMESSAGE_MAX_PARAMETER_COUNT]{};
    unsigned int m_ParameterCount;
    BOOL m_IsValid{};
    char* m_DataPtr;
};
