#pragma once

#include "Common.h"
#include "Lockable.h"
#include "ProcedureEngine.h"
#include "IClient.h"

// ReSharper disable CppPossiblyUninitializedMember
class Engine : public Singleton<Engine>, public Lockable {
public:
    Engine() = default;
    ~Engine() = default;

    void initialize(IClient *client);

    void start();
    void stop();

    void setClientUIDMode(std::string clientUID, CLIENTUID_MODE_PAIR pair);
    CLIENTUID_MODE_PAIR getClientUIDMode(std::string clientUID);

    void setClientDBIDMessage(uint64 clientUID, std::string message);
    std::string getClientDBIDMessage(uint64 clientUID);

    DECLARE_MEMBER(IServer *, PipeManager)
    DECLARE_MEMBER(ProcedureEngine *, ProcedureEngine)
    DECLARE_MEMBER(IClient *, Client)
    DECLARE_MEMBER(STATE, State)
    DECLARE_MEMBER_PRIVATE(CLIENTUID_MODE_MAP, ClientUIDModeMap)
    DECLARE_MEMBER_PRIVATE(CLIENTDBID_MESSAGE_MAP, ClientDBIDMessageMap)
};
// ReSharper restore CppPossiblyUninitializedMember

