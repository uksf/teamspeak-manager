#pragma once

#include "ManagerCommon.h"
#include "../Common/Singleton.h"
#include "../Common/Lockable.h"
#include "../SignalrClient/ISignalrClient.h"
#include "../SignalrClient/ProcedureEngine.h"
#include "IClient.h"

class Engine final : public Singleton<Engine>, public Lockable {
public:
    Engine() = default;
    ~Engine() = default;

    void initialize();

    void start();
    void stop();

	void procShutdown();

	void handleClient(uint64 serverConnectionHandlerID, anyID clientID, uint64 newChannelID, int visibility);

	std::string getClientUID(uint64 serverConnectionHandlerID, anyID clientID);
	MAP_DBID_VALUE getDBIDMapValue(MAP_DBID_KEY key);
	MAP_UID_VALUE getUIDMapValue(MAP_UID_KEY key);
	DBID_QUEUE_MODE getFromCallbackQueue(MAP_UID_KEY key);

	void addToCallbackQueue(MAP_UID_KEY key, DBID_QUEUE_MODE mode);

	void updateClientChannel(uint64 serverConnectionHandlerID, std::string clientUID, uint64 newChannelID);
	void updateOrSetDBIDMapValue(MAP_DBID_KEY key, std::string newClientUID);
	void updateOrSetUIDMapValue(MAP_UID_KEY key, uint64 newDBID, anyID newClientID, std::string newClientName, uint64 newChannelID, std::string newChannelName);
	void updateUIDMapChannelName(uint64 channelID, std::string newChannelName);

	void deleteUIDMapValue(MAP_UID_KEY key);
	void deleteDBIDMapValue(MAP_DBID_KEY key);
	void deleteIDMapValue(MAP_ID_KEY key);

    DECLARE_MEMBER(ISignalrClient*, SignalrClient)
    DECLARE_MEMBER(ProcedureEngine *, ProcedureEngine)
    DECLARE_MEMBER(IClient *, Client)
    DECLARE_MEMBER(STATE, State)

    DECLARE_MEMBER_PRIVATE(MAP_UID, UIDMap)
    DECLARE_MEMBER_PRIVATE(MAP_DBID, DBIDMap)
    DECLARE_MEMBER_PRIVATE(MAP_ID, IDMap)

private:
	void initaliseClientMaps();
	void sendClientsUpdate();

	MAP_ID_VALUE getIDMapValue(MAP_ID_KEY key);

	void updateOrSetIDMapValue(MAP_ID_KEY key, std::string newClientUID);

	DBID_QUEUE m_DBIDCallbackQueue;
};
