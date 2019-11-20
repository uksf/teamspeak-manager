#pragma once

#include "ManagerCommon.h"
#include "../Common/Singleton.h"

class Data final : public Singleton<Data> {
public:
	Data() = default;
	~Data() = default;

	void initialize();

	void resetClientMaps();
	void populateClientMaps();
	void handleClient(anyID clientID, uint64 newChannelID, int visibility);

	static int checkIfBlacklisted(char* name);
	void checkClientServerGroups(const char* clientUniqueIdentity);

	void onClientDBIDfromUID(char* uniqueClientIdentifier, uint64 clientDatabaseID);
	void onClientMoveMoved(anyID clientID, uint64 newChannelID);
	void onClientKickFromChannel(anyID clientID, uint64 newChannelID);
	void onClientKickFromServer(anyID clientID);
	void onClientBanFromServer(anyID clientID);
	void onClientMoveTimeout(anyID clientID);
	void onClientDisplayName(anyID clientID);

	MAP_DBID_VALUE getDBIDMapValue(MAP_DBID_KEY key);
	std::string getClientUID(anyID clientID);
	MAP_UID_VALUE getUIDMapValue(MAP_UID_KEY key);
	DBID_QUEUE_MODE getFromCallbackQueue(MAP_UID_KEY key);

	void addToCallbackQueue(MAP_UID_KEY key, DBID_QUEUE_MODE mode);

	void updateClientChannel(std::string clientUID, uint64 newChannelID);
	void updateOrSetDBIDMapValue(MAP_DBID_KEY key, std::string newClientUID);
	void updateUIDMapChannelName(uint64 channelID, std::string newChannelName);
	void updateOrSetUIDMapValue(MAP_UID_KEY key, uint64 newDBID, anyID newClientID, std::string newClientName, uint64 newChannelID, std::string newChannelName);

	void deleteDBIDMapValue(MAP_DBID_KEY key);
	void deleteIDMapValue(MAP_ID_KEY key);
	void deleteUIDMapValue(MAP_UID_KEY key);

	DECLARE_MEMBER_PRIVATE(MAP_UID, UIDMap)
	DECLARE_MEMBER_PRIVATE(MAP_DBID, DBIDMap)
	DECLARE_MEMBER_PRIVATE(MAP_ID, IDMap)

private:
	DBID_QUEUE m_DBIDCallbackQueue;

	MAP_ID_VALUE getIDMapValue(MAP_ID_KEY key);

	void updateOrSetIDMapValue(MAP_ID_KEY key, std::string newClientUID);

	void sendClientsUpdate();
};
