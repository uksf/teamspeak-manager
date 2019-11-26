#include "Data.h"
#include "Engine.h"

extern TS3Functions ts3Functions;

void Data::initialize() {
	logTSMessage("Data: Initialising");
    this->resetClientMaps();
	logTSMessage("Data: Initialization complete");
}

void Data::resetClientMaps() {
	logTSMessage("Data: Reset client lists tid %u", std::this_thread::get_id());
    this->m_UIDMap.clear();
    this->m_DBIDMap.clear();
    this->m_IDMap.clear();
    this->m_DBIDCallbackQueue.clear();
}

void Data::populateClientMaps() {
    this->resetClientMaps();
    logTSMessage("Data: Populating client list");
    anyID* clients;
    if (ts3Functions.getClientList(ts3Functions.getCurrentServerConnectionHandlerID(), &clients) != ERROR_ok) {
        logTSMessage("Data: Failed getting client list");
        return;
    }
    while (*clients) {
        const anyID clientID = *clients;
        clients++;
        uint64 channelID;
        ts3Functions.getChannelOfClient(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, &channelID);
        this->handleClient(clientID, channelID, ENTER_VISIBILITY);
    }
}

void Data::handleClient(const anyID clientID, const uint64 newChannelID, const int visibility) {
    logTSMessage("Data: handle client %d tid: %u", clientID, std::this_thread::get_id());
    const std::string clientUID = this->getClientUID(clientID);
    if (clientUID.empty()) return;

    switch (visibility) {
    case ENTER_VISIBILITY: {
        logTSMessage("Data: Client joined: %d, %s", clientID, clientUID.c_str());
        char* clientName;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
            logTSMessage("Data: Failed getting client name");
            return;
        }
        this->updateOrSetUIDMapValue(clientUID, NULL_UINT, clientID, clientName, newChannelID, "");
        this->updateClientChannel(clientUID, newChannelID);
        this->addToCallbackQueue(clientUID, DBID_QUEUE_MODE::GROUPS);
        ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUID.c_str(), nullptr);
        break;
    }
    case RETAIN_VISIBILITY: {
        logTSMessage("Data: Client moved: %d, %s", clientID, clientUID.c_str());
        char* clientName;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
            logTSMessage("Failed getting client name");
            return;
        }
        this->updateOrSetUIDMapValue(clientUID, NULL_UINT, clientID, clientName, newChannelID, "");
        this->updateClientChannel(clientUID, newChannelID);
        const MAP_UID_VALUE value = this->getUIDMapValue(clientUID);
        if (value.clientDBID == NULL_UINT) {
            logTSMessage("Data: No DBID stored in UID map for: %s", clientUID.c_str());
            ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUID.c_str(), nullptr);
        } else {
            logTSMessage("Data: DBID found in UID map for: %s", clientUID.c_str());
        }
        break;
    }
    case LEAVE_VISIBILITY: {
        logTSMessage("Data: Client left: %d, unsetting uid %s", clientID, clientUID.c_str());
        this->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
        this->deleteIDMapValue(clientID);
        break;
    }
    default:
        logTSMessage("Data: Visibility error", clientID);
    }
}

int Data::checkIfBlacklisted(char* name) {
    if (strncmp(name, "serveradmin", 40) == 0) return 1;
    if (strncmp(name, "wukoIwZ1SPRWqrVlxyxmZypjpME=", 40) == 0) return 1;
    return 0;
}

void Data::checkClientServerGroups(const char* clientUniqueIdentity) {
    const auto value = this->getUIDMapValue(clientUniqueIdentity);
    if (!value.invalid) {
        if (value.clientDBID != NULL_UINT) {
            ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), value.clientDBID, nullptr);
        } else {
            this->addToCallbackQueue(clientUniqueIdentity, DBID_QUEUE_MODE::GROUPS);
            ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUniqueIdentity, nullptr);
        }
    } else {
        this->addToCallbackQueue(clientUniqueIdentity, DBID_QUEUE_MODE::GROUPS);
        ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUniqueIdentity, nullptr);
    }
}

void Data::onClientDBIDfromUID(const std::string clientUID, const uint64 clientDatabaseID) {
    logTSMessage("Data: Client DBID %llu retrieved for %s tid %u", clientDatabaseID, clientUID.c_str(), std::this_thread::get_id());
    this->updateOrSetDBIDMapValue(clientDatabaseID, clientUID);
    this->updateOrSetUIDMapValue(clientUID, clientDatabaseID, NULL_ANYID, "", NULL_UINT, "");

    const DBID_QUEUE_MODE callback = this->getFromCallbackQueue(clientUID);
    if (callback == DBID_QUEUE_MODE::UNSET) {
        return;
    }
    logTSMessage("Data: Found callback to handle in DBID queue for UID %s", clientUID.c_str());
    const MAP_UID_VALUE mapUIDValue = this->getUIDMapValue(clientUID);
    switch (callback) {
    case DBID_QUEUE_MODE::GROUPS:
        logTSMessage("Data: Handling Groups callback");
        ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), mapUIDValue.clientDBID, nullptr);
        return;
    default: break;
    }
    logTSMessage("Data: Invalid callback mode");
}

void Data::onClientMoveMoved(const anyID clientID, const uint64 newChannelID) {
    // update channelID and channelName
    const std::string clientUID = this->getClientUID(clientID);
    if (clientUID.empty()) return;
    this->updateClientChannel(clientUID, newChannelID);
}

void Data::onClientKickFromChannel(const anyID clientID, const uint64 newChannelID) {
    // update channelID and channelName
    const std::string clientUID = this->getClientUID(clientID);
    if (clientUID.empty()) return;
    this->updateClientChannel(clientUID, newChannelID);
}

void Data::onClientKickFromServer(const anyID clientID) {
    // unset clientID and channelID, delete from ID map
    const std::string clientUID = this->getClientUID(clientID);
    if (clientUID.empty()) return;
    this->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
    this->deleteIDMapValue(clientID);
}

void Data::onClientBanFromServer(const anyID clientID) {
    // delete from all maps
    const std::string clientUID = this->getClientUID(clientID);
    if (clientUID.empty()) return;
    const auto value = this->getUIDMapValue(clientUID);
    if (value.clientDBID != NULL_UINT) {
        this->deleteDBIDMapValue(value.clientDBID);
    }
    this->deleteUIDMapValue(clientUID);
    this->deleteIDMapValue(clientID);
}

void Data::onClientMoveTimeout(const anyID clientID) {
    // unset clientID and channelID, delete from ID map
    const std::string clientUID = this->getClientUID(clientID);
    if (clientUID.empty()) return;
    this->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
    this->deleteIDMapValue(clientID);
}

void Data::onClientDisplayName(const anyID clientID) {
    // update clientName
    const std::string clientUID = this->getClientUID(clientID);
    if (clientUID.empty()) return;
    char* clientName;
    if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
        logTSMessage("Data: Failed getting client name");
        return;
    }
    this->updateOrSetUIDMapValue(clientUID, NULL_UINT, NULL_ANYID, clientName, NULL_UINT, "");
}

std::string Data::getClientUID(const anyID clientID) {
    logTSMessage("Data: Get client UID from ID %d tid: %u", clientID, std::this_thread::get_id());
    char* clientUID;
    std::string clientUIDString = this->getIDMapValue(clientID);
    if (clientUIDString.empty()) {
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) != ERROR_ok) {
            logTSMessage("Data: Failed to get client UID: %d. Won't continue", clientID);
            return "";
        }
        if (this->checkIfBlacklisted(clientUID)) return "";
        logTSMessage("Data: Client UID not found in ID map, setting %d as %s", clientID, clientUID);
        this->updateOrSetIDMapValue(clientID, clientUID);
    } else {
        logTSMessage("Data: Client UID found in ID map, getting %d as %s", clientID, clientUIDString.c_str());
        return clientUIDString;
    }
    return clientUID;
}

MAP_DBID_VALUE Data::getDBIDMapValue(const MAP_DBID_KEY key) {
    const auto iterator = this->m_DBIDMap.find(key);
    if (iterator != this->m_DBIDMap.end()) {
		logTSMessage("Data: Get DBID map value found '%s' by key '%llu'", iterator->second.c_str(), key);
        return iterator->second;
    }
	logTSMessage("Data: Get DBID map value not found for key '%llu'", key);
    return "";
}

MAP_UID_VALUE Data::getUIDMapValue(const MAP_UID_KEY key) {
    const auto iterator = this->m_UIDMap.find(key);
    if (iterator != this->m_UIDMap.end()) {
        return iterator->second;
    }
    return MAP_UID_VALUE{};
}

DBID_QUEUE_MODE Data::getFromCallbackQueue(const MAP_UID_KEY key) {
    const auto iterator = this->m_DBIDCallbackQueue.find(key);
    if (iterator != this->m_DBIDCallbackQueue.end()) {
        return iterator->second;
    }
    return DBID_QUEUE_MODE::UNSET;
}

void Data::addToCallbackQueue(MAP_UID_KEY key, DBID_QUEUE_MODE mode) {
    this->m_DBIDCallbackQueue.emplace(key, mode);
}

void Data::updateClientChannel(const std::string clientUID, const uint64 newChannelID) {
    char* channelName;
    if (ts3Functions.getChannelVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), newChannelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
        logTSMessage("Data: Failed getting channel name");
        return;
    }
    this->updateOrSetUIDMapValue(clientUID, NULL_UINT, NULL_ANYID, "", newChannelID, channelName);
}

void Data::updateOrSetDBIDMapValue(MAP_DBID_KEY key, std::string newClientUID) {
    const auto iterator = this->m_DBIDMap.find(key);
    if (iterator != this->m_DBIDMap.end()) {
        logTSMessage("Data: Updating client DBID %llu for %s", key, newClientUID.c_str());
        if (!newClientUID.empty()) {
            iterator->second = newClientUID;
        }
    } else {
        logTSMessage("Data: Emplacing client DBID %llu for %s", key, newClientUID.c_str());
        this->m_DBIDMap.emplace(key, newClientUID);
    }
}

void Data::updateOrSetUIDMapValue(MAP_UID_KEY key, const uint64 newDBID, const anyID newClientID, const std::string newClientName, const uint64 newChannelID,
                                  const std::string newChannelName) {
    const auto iterator = this->m_UIDMap.find(key);
    if (iterator != this->m_UIDMap.end()) {
        if (newDBID != NULL_UINT) {
			logTSMessage("Data: Updating client UID %s DBID to %llu.", key.c_str(), newDBID);
            iterator->second.clientDBID = newDBID;
        }
        if (newClientID != NULL_ANYID) {
			logTSMessage("Data: Updating client UID %s clientID to %llu.", key.c_str(), newClientID);
            iterator->second.clientID = newClientID;
        }
        if (!newClientName.empty()) {
			logTSMessage("Data: Updating client UID %s client name to %s.", key.c_str(), newClientName.c_str());
            iterator->second.clientName = newClientName;
        }
        if (newChannelID != NULL_UINT) {
			logTSMessage("Data: Updating client UID %s channel id to %llu.", key.c_str(), newChannelID);
            iterator->second.channelID = newChannelID;
        }
        if (!newChannelName.empty()) {
			logTSMessage("Data: Updating client UID %s channel name to %s.", key.c_str(), newChannelName.c_str());
            iterator->second.channelName = newChannelName;
        }
    } else {
        logTSMessage("Data: Emplacing client UID %s: %llu, %llu, %s, %llu, %s.", key.c_str(), newDBID, newClientID, newClientName.c_str(), newChannelID, newChannelName.c_str());
        this->m_UIDMap.emplace(key, MAP_UID_VALUE{MAP_UID_VALUE(newDBID, newClientID, newClientName, newChannelID, newChannelName)});
    }
    this->sendClientsUpdate();
}

void Data::updateUIDMapChannelName(const uint64 channelID, const std::string newChannelName) {
    for (auto iterator = this->m_UIDMap.begin(); iterator != this->m_UIDMap.end(); ++iterator) {
        auto value = iterator->second;
        if (value.channelID == channelID) {
            logTSMessage("Data: Channel edited, updating name for: %s", iterator->first.c_str());
            value.channelName = newChannelName;
        }
    }
    this->sendClientsUpdate();
}

void Data::deleteUIDMapValue(const MAP_UID_KEY key) {
    const auto iterator = this->m_UIDMap.find(key);
    if (iterator != this->m_UIDMap.end()) {
        this->m_UIDMap.erase(iterator);
    }
    this->sendClientsUpdate();
}

void Data::deleteDBIDMapValue(const MAP_DBID_KEY key) {
    const auto iterator = this->m_DBIDMap.find(key);
    if (iterator != this->m_DBIDMap.end()) {
        this->m_DBIDMap.erase(iterator);
    }
}

void Data::deleteIDMapValue(const MAP_ID_KEY key) {
    const auto iterator = this->m_IDMap.find(key);
    if (iterator != this->m_IDMap.end()) {
        this->m_IDMap.erase(iterator);
    }
}

MAP_ID_VALUE Data::getIDMapValue(const MAP_ID_KEY key) {
    const auto iterator = this->m_IDMap.find(key);
    if (iterator != this->m_IDMap.end()) {
        return iterator->second;
    }
    return "";
}

void Data::updateOrSetIDMapValue(MAP_ID_KEY key, std::string newClientUID) {
    const auto iterator = this->m_IDMap.find(key);
    if (iterator != this->m_IDMap.end()) {
        logTSMessage("Data: Updating client ID %d", key);
        if (!newClientUID.empty()) {
            iterator->second = newClientUID;
        }
    } else {
        logTSMessage("Data: Emplacing client ID %d", key);
        this->m_IDMap.emplace(key, newClientUID);
    }
}

void Data::sendClientsUpdate() {
    std::vector<signalr::value> clients;
    for (const auto& pair : this->m_UIDMap) {
        const auto value = pair.second;
        if (value.clientID != UNSET_ANYID) {
			logTSMessage("Data: Will send: %llu, %s, %llu, %s.", value.clientDBID, value.clientName.c_str(), value.channelID, value.channelName.c_str());
            std::map<std::string, signalr::value> clientMap;
            clientMap.insert(std::pair<std::string, signalr::value>("clientDBID", signalr::value(static_cast<double>(value.clientDBID))));
            clientMap.insert(std::pair<std::string, signalr::value>("clientName", value.clientName));
            clientMap.insert(std::pair<std::string, signalr::value>("channelID", signalr::value(static_cast<double>(value.channelID))));
            clientMap.insert(std::pair<std::string, signalr::value>("channelName", value.channelName));
            clients.emplace_back(clientMap);
        }
    }
    Engine::getInstance()->addToSendQueue(SERVER_MESSAGE_TYPE::CLIENTS, signalr::value(clients));
}
