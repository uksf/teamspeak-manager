#include <teamspeak/public_errors.h>
#include "Plugin.h"
#include "Engine.h"
#include "PipeManager.h"
#include "ProcPing.h"
#include "ProcAssignServerGroup.h"
#include "ProcUnassignServerGroup.h"
#include "ProcUpdateServerGroups.h"
#include "ProcGetServerSnapshot.h"
#include "ProcSendMessageToClient.h"
#include "ProcShutdown.h"
#include "ProcGetOnlineClients.h"
#include "ProcInitaliseClientMaps.h"

extern TS3Functions ts3Functions;

void Engine::initialize(IClient* client) {
    this->setClient(client);
    this->m_PipeManager = new PipeManager();
    this->m_ProcedureEngine = new ProcedureEngine();

    this->getProcedureEngine()->addProcedure(new ProcPing());
    this->getProcedureEngine()->addProcedure(new ProcInitaliseClientMaps());
    this->getProcedureEngine()->addProcedure(new ProcShutdown());
    this->getProcedureEngine()->addProcedure(new ProcUpdateServerGroups());
    this->getProcedureEngine()->addProcedure(new ProcAssignServerGroup());
    this->getProcedureEngine()->addProcedure(new ProcUnassignServerGroup());
    this->getProcedureEngine()->addProcedure(new ProcGetServerSnapshot());
    this->getProcedureEngine()->addProcedure(new ProcSendMessageToClient());
    this->getProcedureEngine()->addProcedure(new ProcGetOnlineClients());
}

void Engine::start() {
    logTSMessage("Engine starting up");
    this->m_UIDMap.clear();
    this->m_DBIDMap.clear();
    this->m_IDMap.clear();
    if (this->getPipeManager()) {
        this->getPipeManager()->initialize();
    }
    this->setState(STATE_RUNNING);
    logTSMessage("Engine startup complete");
}

void Engine::stop() {
    logTSMessage("Engine shutting down");
    this->setState(STATE_STOPPING);
    if (this->getProcedureEngine()) {
        this->getProcedureEngine()->stopWorker();
    }
    if (this->getPipeManager()) {
        this->getPipeManager()->shutdown();
    }
    this->m_UIDMap.clear();
    this->m_DBIDMap.clear();
    this->m_IDMap.clear();
    this->setState(STATE_STOPPED);
    logTSMessage("Engine shutdown complete");
}

void Engine::initaliseClientMaps() {
    logTSMessage("Initialising client list");
    anyID* clients;
    if (ts3Functions.getClientList(ts3Functions.getCurrentServerConnectionHandlerID(), &clients) != ERROR_ok) {
        logTSMessage("Failed getting client list");
        return;
    }
    while (*clients) {
        const anyID clientID = *clients;
        clients++;
        uint64 channelID;
        ts3Functions.getChannelOfClient(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, &channelID);
        this->handleClient(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, channelID, ENTER_VISIBILITY);
    }
}

std::string Engine::getClientUID(uint64 serverConnectionHandlerID, anyID clientID) {
    char* clientUID;
    std::string clientUIDString = this->getIDMapValue(clientID);
    if (clientUIDString.empty()) {
        if (ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) != ERROR_ok) {
            logTSMessage("Failed to get client UID: %d. Won't continue", clientID);
            return "";
        }
        if (this->getClient()->checkIfBlacklisted(clientUID)) return "";
        logTSMessage("Client UID not found in ID map, setting %d as %s", clientID, clientUID);
        this->updateOrSetIDMapValue(clientID, clientUID);
    } else {
        logTSMessage("Client UID found in ID map, getting %d as %s", clientID, clientUIDString.c_str());
        return clientUIDString;
    }
    return clientUID;
}

void Engine::updateClientChannel(uint64 serverConnectionHandlerID, std::string clientUID, uint64 newChannelID) {
    char* channelName;
    if (ts3Functions.getChannelVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), newChannelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
        logTSMessage("Failed getting channel name");
        return;
    }
    this->updateOrSetUIDMapValue(clientUID, NULL_UINT, NULL_ANYID, "", newChannelID, channelName);
}

void Engine::handleClient(uint64 serverConnectionHandlerID, anyID clientID, uint64 newChannelID, int visibility) {
    // connected
    // store clientID, UID, clientName, channelID, channelName
    // add UID to DBID event queue in server group check mode
    // request DBID

    // move
    // update channelID and channelName
    // check we have DBID

    // disconnected
    // unset clientID and channelID

    std::string clientUID = getClientUID(serverConnectionHandlerID, clientID);
    if (clientUID.empty()) return;

    switch (visibility) {
    case ENTER_VISIBILITY: {
        logTSMessage("Client joined: %d", clientID);
        char* clientName;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
            logTSMessage("Failed getting client name");
            return;
        }
        this->updateOrSetUIDMapValue(clientUID, NULL_UINT, clientID, clientName, newChannelID, "");
        updateClientChannel(serverConnectionHandlerID, clientUID, newChannelID);
        this->addToCallbackQueue(clientUID, DBID_QUEUE_MODE_GROUPS);
        ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID.c_str(), nullptr);
        break;
    }
    case RETAIN_VISIBILITY: {
        logTSMessage("Client moved: %d", clientID);
        char* clientName;
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_NICKNAME, &clientName) != ERROR_ok) {
            logTSMessage("Failed getting client name");
            return;
        }
        this->updateOrSetUIDMapValue(clientUID, NULL_UINT, clientID, clientName, newChannelID, "");
        updateClientChannel(serverConnectionHandlerID, clientUID, newChannelID);
        const auto value = this->getUIDMapValue(clientUID);
        if (value.clientDBID == NULL_UINT) {
            logTSMessage("No DBID stored in UID map for: %s", clientUID.c_str());
            ts3Functions.requestClientDBIDfromUID(serverConnectionHandlerID, clientUID.c_str(), nullptr);
        } else {
            logTSMessage("DBID found in UID map for: %s", clientUID.c_str());
        }
        break;
    }
    case LEAVE_VISIBILITY: {
        logTSMessage("Client left: %d, unsetting uid %s", clientID, clientUID.c_str());
        this->updateOrSetUIDMapValue(clientUID, NULL_UINT, UNSET_ANYID, "", NULL_UINT, "");
        this->deleteIDMapValue(clientID);
        break;
    }
    default:
        logTSMessage("Visibility error", clientID);
    }
}

MAP_UID_VALUE Engine::getUIDMapValue(MAP_UID_KEY key) {
    LOCK(this);
    const auto iterator = this->m_UIDMap.find(key);
    if (iterator != this->m_UIDMap.end()) {
        UNLOCK(this);
        return iterator->second;
    }
    UNLOCK(this);
    return MAP_UID_VALUE{};
}

void Engine::updateOrSetUIDMapValue(MAP_UID_KEY key, uint64 newDBID, anyID newClientID, std::string newClientName, uint64 newChannelID, std::string newChannelName) {
    LOCK(this);
    const auto iterator = this->m_UIDMap.find(key);
    if (iterator != this->m_UIDMap.end()) {
        logTSMessage("Updating client UID %s", key.c_str());
        if (newDBID != NULL_UINT) {
            iterator->second.clientDBID = newDBID;
        }
        if (newClientID != NULL_ANYID) {
            iterator->second.clientID = newClientID;
        }
        if (!newClientName.empty()) {
            iterator->second.clientName = newClientName;
        }
        if (newChannelID != NULL_UINT) {
            iterator->second.channelID = newChannelID;
        }
        if (!newChannelName.empty()) {
            iterator->second.channelName = newChannelName;
        }
    } else {
        logTSMessage("Emplacing client UID %s", key.c_str());
        this->m_UIDMap.emplace(key, MAP_UID_VALUE{MAP_UID_VALUE(newDBID, newClientID, newClientName, newChannelID, newChannelName)});
    }
    UNLOCK(this);
}

void Engine::updateUIDMapChannelName(uint64 channelID, std::string newChannelName) {
    LOCK(this);
    for (auto iterator = this->m_UIDMap.begin(); iterator != this->m_UIDMap.end(); ++iterator) {
        auto value = iterator->second;
        if (value.channelID == channelID) {
            logTSMessage("Channel edited, updating name for: %s", iterator->first.c_str());
            value.channelName = newChannelName;
        }
    }
    UNLOCK(this);
}

void Engine::deleteUIDMapValue(MAP_UID_KEY key) {
    LOCK(this);
    const auto iterator = this->m_UIDMap.find(key);
    if (iterator != this->m_UIDMap.end()) {
        this->m_UIDMap.erase(iterator);
    }
    UNLOCK(this);
}

MAP_DBID_VALUE Engine::getDBIDMapValue(MAP_DBID_KEY key) {
    LOCK(this);
    const auto iterator = this->m_DBIDMap.find(key);
    if (iterator != this->m_DBIDMap.end()) {
        UNLOCK(this);
        return iterator->second;
    }
    UNLOCK(this);
    return "";
}

void Engine::updateOrSetDBIDMapValue(MAP_DBID_KEY key, std::string newClientUID) {
    LOCK(this);
    const auto iterator = this->m_DBIDMap.find(key);
    if (iterator != this->m_DBIDMap.end()) {
        logTSMessage("Updating client DBID %llu for %s", key, newClientUID.c_str());
        if (!newClientUID.empty()) {
            iterator->second = newClientUID;
        }
    } else {
        logTSMessage("Emplacing client DBID %llu for %s", key, newClientUID.c_str());
        this->m_DBIDMap.emplace(key, newClientUID);
    }
    UNLOCK(this);
}

void Engine::deleteDBIDMapValue(MAP_DBID_KEY key) {
    LOCK(this);
    const auto iterator = this->m_DBIDMap.find(key);
    if (iterator != this->m_DBIDMap.end()) {
        this->m_DBIDMap.erase(iterator);
    }
    UNLOCK(this);
}

MAP_ID_VALUE Engine::getIDMapValue(MAP_ID_KEY key) {
    LOCK(this);
    const auto iterator = this->m_IDMap.find(key);
    if (iterator != this->m_IDMap.end()) {
        UNLOCK(this);
        return iterator->second;
    }
    UNLOCK(this);
    return "";
}

void Engine::updateOrSetIDMapValue(MAP_ID_KEY key, std::string newClientUID) {
    LOCK(this);
    const auto iterator = this->m_IDMap.find(key);
    if (iterator != this->m_IDMap.end()) {
        logTSMessage("Emplacing client ID %d", key);
        if (!newClientUID.empty()) {
            iterator->second = newClientUID;
        }
    } else {
        logTSMessage("Emplacing client ID %d", key);
        this->m_IDMap.emplace(key, newClientUID);
    }
    UNLOCK(this);
}

void Engine::deleteIDMapValue(MAP_ID_KEY key) {
    LOCK(this);
    const auto iterator = this->m_IDMap.find(key);
    if (iterator != this->m_IDMap.end()) {
        this->m_IDMap.erase(iterator);
    }
    UNLOCK(this);
}

void Engine::addToCallbackQueue(MAP_UID_KEY key, DBID_QUEUE_MODE mode) {
    LOCK(this);
    this->m_DBIDCallbackQueue.emplace(key, mode);
    UNLOCK(this);
}

DBID_QUEUE_MODE Engine::getFromCallbackQueue(MAP_UID_KEY key) {
    LOCK(this);
    const auto iterator = this->m_DBIDCallbackQueue.find(key);
    if (iterator != this->m_DBIDCallbackQueue.end()) {
        UNLOCK(this);
        return iterator->second;
    }
    UNLOCK(this);
    return DBID_QUEUE_MODE_UNSET;
}

void Engine::sendServerSnapshot() {
    LOCK(this);
    for (auto iterator = this->m_UIDMap.begin(); iterator != this->m_UIDMap.end(); ++iterator) {
        const auto value = iterator->second;
        if (value.clientID == UNSET_ANYID) continue;
        const int lastClient = std::next(iterator) == this->m_UIDMap.end() ? 1 : 0;
        this->getPipeManager()->sendMessage(
            TextMessage::formatNewMessage(const_cast<char*>("StoreServerSnapshot"), const_cast<char*>("%d|%s|%d|%s|%d"), value.clientDBID, value.clientName.c_str(),
                                          value.channelID,
                                          value.channelName.c_str(), lastClient));
    }
    UNLOCK(this);
}

void Engine::sendOnlineClients() {
    LOCK(this);
    for (auto iterator = this->m_UIDMap.begin(); iterator != this->m_UIDMap.end(); ++iterator) {
        const auto value = iterator->second;
        logTSMessage("Online? client ID %d", value.clientID);
        if (value.clientID == UNSET_ANYID) continue;
        const int lastClient = std::next(iterator) == this->m_UIDMap.end() ? 1 : 0;
        this->getPipeManager()->sendMessage(
            TextMessage::formatNewMessage(const_cast<char*>("UpdateOnlineClients"), const_cast<char*>("%d|%s|%d|%s|%d"), value.clientDBID, value.clientName.c_str(),
                                          value.channelID,
                                          value.channelName.c_str(), lastClient));
    }
    UNLOCK(this);
}
