#include "TextMessage.h"
#include "IMessage.h"
#include <string>

TextMessage::TextMessage(char* value, const size_t len) {
    this->m_ParameterCount = 0;
    this->m_RpcProcedureName = nullptr;
    this->m_DataPtr = nullptr;
    this->parse(value, len);
}

TextMessage::~TextMessage() {
    for (auto& m_Parameter : this->m_Parameters) {
        delete m_Parameter;
    }
    delete this->m_RpcProcedureName;
    delete this->m_Data;
    if (this->m_DataPtr)
        LocalFree(this->m_DataPtr);
}

BOOL TextMessage::isValid() const {
    return this->m_IsValid;
}

void TextMessage::parse(char* value, const size_t len) {
    size_t x;
    this->m_RpcProcedureName = nullptr;
    this->m_Data = nullptr;
    memset(this->m_Parameters, 0x00, sizeof this->m_Parameters);
    this->m_IsValid = FALSE;
    this->m_ParameterCount = 0;
    if (!value) {
        this->m_IsValid = FALSE;
        return;
    }
    for (x = 0; x < len; x++) {
        if (!__isascii(value[x]) && value[x] != 0x00) {
            this->m_IsValid = FALSE;
        }
        if (value[x] == 0x00) break;
    }
    const size_t length = strlen(value) + 1;
    if (length < 3) {
        this->m_IsValid = FALSE;
        return;
    }
    this->m_DataPtr = static_cast<char *>(LocalAlloc(0, length));
    memcpy(this->m_DataPtr, value, length);
    this->m_DataPtr[length - 1] = 0x00;
    this->m_Data = new std::string(this->m_DataPtr);
    x = this->m_Data->find_first_of(":");
    if (x < 2 || x > 1000000 || x == std::string::npos) {
        this->m_IsValid = FALSE;
        return;
    }
    const std::string procedureName = this->m_Data->substr(0, x);
    this->m_RpcProcedureName = new std::string(procedureName);
    if (x == this->m_Data->length() - 1) {
        this->m_IsValid = TRUE;
        return;
    }
    std::string t(this->m_Data->substr(this->m_Data->find_first_of(":") + 1, this->m_Data->length() - this->m_Data->find_first_of(":") + 1));
    int pParamCount = 0;
    for (x = 0; x < TEXTMESSAGE_MAX_PARAMETER_COUNT; x++) {
        if (t.length() < 1)
            break;
        if (t.length() > 1 && t.find("<null>") == std::string::npos && t.find('|') != std::string::npos) {
            this->m_Parameters[x] = new std::string(t.substr(0, t.find('|')));
            pParamCount += 1;
        } else if (t.find('|') == std::string::npos) {
            this->m_IsValid = TRUE;
            this->m_Parameters[x] = new std::string(t.substr(0, t.length()));
            pParamCount += 1;
            break;
        }
        if (this->m_Parameters[x] == nullptr) {
            this->m_Parameters[x] = new std::string("");
        } else {
            if (this->m_Parameters[x]->length() < 1) {
                delete this->m_Parameters[x];
                this->m_Parameters[x] = new std::string("");
            }
        }
        t = t.substr(t.find('|') + 1, t.length() - 1);
    }

    this->m_ParameterCount = pParamCount;
    this->m_IsValid = TRUE;
}

int TextMessage::getParameterAsInt(const unsigned int index) {
    std::string value = this->getParameter(index);
    if (value.length() > 0) {
        return int(std::stoi(value));
    }
    return 0xFFFFFFFF;
}

std::string TextMessage::getParameter(const unsigned int index) {
    if (index > this->m_ParameterCount) {
        return nullptr;
    }
    if (this->m_Parameters[index] == nullptr) {
        return nullptr;
    }
    const auto result = *this->m_Parameters[index];
    return result;
}

char* TextMessage::getProcedureName() {
    if (this->m_RpcProcedureName) {
        return const_cast<char *>(this->m_RpcProcedureName->c_str());
    }
    return nullptr;
}

IMessage* TextMessage::formatNewMessage(char* procedureName, char* format, ...) {
    char buffer[TEXTMESSAGE_BUFSIZE];
    va_list va;
    if (!procedureName) {
        return nullptr;
    }
    auto* finalBuffer = static_cast<char *>(LocalAlloc(LPTR, TEXTMESSAGE_BUFSIZE));
    if (!buffer) {
        return nullptr;
    }
    buffer[0] = 0x00;
    _snprintf_s(finalBuffer, TEXTMESSAGE_BUFSIZE, TEXTMESSAGE_BUFSIZE - 1, "%s:", procedureName);
    va_start(va, format);
    vsprintf_s(buffer, sizeof buffer, format, va);
    va_end(va);
    strcat_s(finalBuffer, TEXTMESSAGE_BUFSIZE, buffer);
    auto msg = new TextMessage(finalBuffer, strlen(finalBuffer) + 1);
    if (!msg->isValid()) {
        delete msg;
        msg = nullptr;
    }
    LocalFree(finalBuffer);

    return static_cast<IMessage *>(msg);
}

IMessage* TextMessage::createNewMessage(char* procedureName, ...) {
    va_list va;
    if (!procedureName) {
        return nullptr;
    }
    auto* buffer = static_cast<char *>(LocalAlloc(LPTR, TEXTMESSAGE_BUFSIZE));
    if (!buffer) {
        return nullptr;
    }
    buffer[0] = 0x00;
    _snprintf_s(buffer, TEXTMESSAGE_BUFSIZE, TEXTMESSAGE_BUFSIZE - 1, "%s:", procedureName);
    va_start(va, procedureName);
    char* ptr = va_arg( va, char * );
    while (ptr != nullptr) {
        strcat_s(buffer, TEXTMESSAGE_BUFSIZE, ptr);
        strcat_s(buffer, TEXTMESSAGE_BUFSIZE, ",");
        ptr = va_arg( va, char * );
    }
    va_end(va);
    buffer = static_cast<char *>(LocalReAlloc(buffer, strlen(buffer) + 1, LMEM_MOVEABLE));
    auto* msg = new TextMessage(buffer, strlen(buffer) + 1);
    if (!msg->isValid()) {
        delete msg;
        msg = nullptr;
    }
    LocalFree(buffer);

    return static_cast<IMessage *>(msg);
}

unsigned int TextMessage::getParameterCount() {
    return this->m_ParameterCount;
}
