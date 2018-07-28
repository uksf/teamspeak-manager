#pragma once

#include "Common.h";
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

    DECLARE_MEMBER(IServer *, PipeManager)
    DECLARE_MEMBER(ProcedureEngine *, ProcedureEngine)
    DECLARE_MEMBER(IClient *, Client)
    DECLARE_MEMBER(STATE, State)
};
// ReSharper restore CppPossiblyUninitializedMember

