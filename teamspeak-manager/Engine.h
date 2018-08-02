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

    MAP_UID_VALUE getUIDMapValue(MAP_UID_KEY key);
    void updateOrSetUIDMapValue(MAP_UID_KEY key, uint64 newDBID, anyID newClientID, std::string newClientName, uint64 newChannelID, std::string newChannelName);
    void updateUIDMapChannelName(uint64 channelID, std::string newChannelName);
    void deleteUIDMapValue(MAP_UID_KEY key);

    MAP_DBID_VALUE getDBIDMapValue(MAP_DBID_KEY key);
    void updateOrSetDBIDMapValue(MAP_DBID_KEY key, std::string newClientUID);
    void deleteDBIDMapValue(MAP_DBID_KEY key);

    MAP_ID_VALUE getIDMapValue(MAP_ID_KEY key);
    void updateOrSetIDMapValue(MAP_ID_KEY key, std::string newClientUID);
    void deleteIDMapValue(MAP_ID_KEY key);

    void addToCallbackQueue(MAP_UID_KEY key, DBID_QUEUE_MODE mode);
    DBID_QUEUE_MODE getFromCallbackQueue(MAP_UID_KEY key);

    void sendServerSnapshot();
    void sendOnlineClients();

    DECLARE_MEMBER(IServer *, PipeManager)
    DECLARE_MEMBER(ProcedureEngine *, ProcedureEngine)
    DECLARE_MEMBER(IClient *, Client)
    DECLARE_MEMBER(STATE, State)

    DECLARE_MEMBER_PRIVATE(MAP_UID, UIDMap)
    DECLARE_MEMBER_PRIVATE(MAP_DBID, DBIDMap)
    DECLARE_MEMBER_PRIVATE(MAP_ID, IDMap)

    DBID_QUEUE m_DBIDCallbackQueue;

private:
    static void initaliseClientMaps();
};
// ReSharper restore CppPossiblyUninitializedMember

