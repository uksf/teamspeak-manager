#pragma once

#include "Common.h";
#include "Lockable.h"
#include "ProcedureEngine.h"
#include "IClient.h"
#include "Log.h"

// ReSharper disable CppPossiblyUninitializedMember
class Engine : public Singleton<Engine>, public Lockable {
public:
    Engine() { g_Log = nullptr; }
    ~Engine() { LOG("* Destroying logging engine."); delete g_Log; }

    void initialize(IClient *client);

    void start();
    void stop();

    DECLARE_MEMBER(IServer *, PipeManager)
    DECLARE_MEMBER(ProcedureEngine *, ProcedureEngine)
    DECLARE_MEMBER(IClient *, Client)
    DECLARE_MEMBER(STATE, State)
};
// ReSharper restore CppPossiblyUninitializedMember

