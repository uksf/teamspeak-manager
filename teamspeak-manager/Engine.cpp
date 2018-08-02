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

extern TS3Functions ts3Functions;

void Engine::initialize(IClient* client) {
    this->setClient(client);
    this->m_PipeManager = new PipeManager();
    this->m_ProcedureEngine = new ProcedureEngine();

    this->getProcedureEngine()->addProcedure(new ProcPing());
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
        this->m_UIDMap.emplace(key, MAP_UID_VALUE{ MAP_UID_VALUE(newDBID, newClientID, newClientName, newChannelID, newChannelName)});
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
        const int lastClient = std::next(iterator) == this->m_UIDMap.end() ? 1 : 0;
        this->getPipeManager()->sendMessage(
            TextMessage::formatNewMessage(const_cast<char*>("StoreServerSnapshot"), const_cast<char*>("%d|%s|%d|%s|%d"), value.clientDBID, value.clientName.c_str(), value.channelID,
                                          value.channelName.c_str(), lastClient));
    }
    UNLOCK(this);
}

void Engine::sendOnlineClients() {
    LOCK(this);
    for (auto iterator = this->m_UIDMap.begin(); iterator != this->m_UIDMap.end(); ++iterator) {
        const auto value = iterator->second;
        const int lastClient = std::next(iterator) == this->m_UIDMap.end() ? 1 : 0;
        this->getPipeManager()->sendMessage(
            TextMessage::formatNewMessage(const_cast<char*>("UpdateOnlineClients"), const_cast<char*>("%d|%s|%d|%s|%d"), value.clientDBID, value.clientName.c_str(), value.channelID,
                                          value.channelName.c_str(), lastClient));
    }
    UNLOCK(this);
}
