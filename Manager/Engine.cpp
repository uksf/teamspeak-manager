#include <sstream>
#include <future>
#include "Engine.h"
#include "../SignalrClient/SignalrClient.h"
#include "ProcAssignServerGroup.h"
#include "ProcUnassignServerGroup.h"
#include "ProcUpdateServerGroups.h"
#include "ProcSendMessageToClient.h"
#include "ProcShutdown.h"
#include "TSClient.h"

extern TS3Functions ts3Functions;

void Engine::initialize() {
    this->setClient(new TSClient());
    this->setSignalrClient(new SignalrClient());
    this->setProcedureEngine(new ProcedureEngine());

    this->getProcedureEngine()->addProcedure(new ProcShutdown());
    this->getProcedureEngine()->addProcedure(new ProcUpdateServerGroups());
    this->getProcedureEngine()->addProcedure(new ProcAssignServerGroup());
    this->getProcedureEngine()->addProcedure(new ProcUnassignServerGroup());
    this->getProcedureEngine()->addProcedure(new ProcSendMessageToClient());
}

void Engine::start() {
    logTSMessage("Engine starting up");
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_UIDMap.clear();
        this->m_DBIDMap.clear();
        this->m_IDMap.clear();
        this->getSignalrClient()->initialize([this]() {
                                                 this->initaliseClientMaps();
                                             }, [this](const ClientMessage message) {
                                                 this->getProcedureEngine()->runProcedure(message);
                                             });
        this->setState(STATE::RUNNING);
    }
    logTSMessage("Engine startup complete");
}

void Engine::stop() {
    logTSMessage("Engine shutting down");
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->setState(STATE::STOPPING);
        if (this->getProcedureEngine()) {
            this->getProcedureEngine()->stopWorker();
        }
        if (this->getSignalrClient()) {
            this->getSignalrClient()->shutdown();
        }
        this->m_UIDMap.clear();
        this->m_DBIDMap.clear();
        this->m_IDMap.clear();
        this->setState(STATE::STOPPED);
    }
    logTSMessage("Engine shutdown complete");
}

void Engine::initaliseClientMaps() {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_UIDMap.clear();
        this->m_DBIDMap.clear();
        this->m_IDMap.clear();
    }
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
        this->handleClient(clientID, channelID, ENTER_VISIBILITY);
    }
}

std::string Engine::getClientUID(anyID clientID) {
    char* clientUID;
    std::string clientUIDString = this->getIDMapValue(clientID);
    if (clientUIDString.empty()) {
        if (ts3Functions.getClientVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), clientID, CLIENT_UNIQUE_IDENTIFIER, &clientUID) != ERROR_ok) {
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

void Engine::updateClientChannel(std::string clientUID, uint64 newChannelID) {
    char* channelName;
    if (ts3Functions.getChannelVariableAsString(ts3Functions.getCurrentServerConnectionHandlerID(), newChannelID, CHANNEL_NAME, &channelName) != ERROR_ok) {
        logTSMessage("Failed getting channel name");
        return;
    }
    this->updateOrSetUIDMapValue(clientUID, NULL_UINT, NULL_ANYID, "", newChannelID, channelName);
}

void Engine::handleClient(anyID clientID, uint64 newChannelID, int visibility) {
    // connected
    // store clientID, UID, clientName, channelID, channelName
    // add UID to DBID event queue in server group check mode
    // request DBID

    // move
    // update channelID and channelName
    // check we have DBID

    // disconnected
    // unset clientID and channelID

    const std::string clientUID = this->getClientUID(clientID);
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
        this->updateClientChannel(clientUID, newChannelID);
        this->addToCallbackQueue(clientUID, DBID_QUEUE_MODE::GROUPS);
        ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUID.c_str(), nullptr);
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
        this->updateClientChannel(clientUID, newChannelID);
        const auto value = this->getUIDMapValue(clientUID);
        if (value.clientDBID == NULL_UINT) {
            logTSMessage("No DBID stored in UID map for: %s", clientUID.c_str());
            ts3Functions.requestClientDBIDfromUID(ts3Functions.getCurrentServerConnectionHandlerID(), clientUID.c_str(), nullptr);
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
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        const auto iterator = this->m_UIDMap.find(key);
        if (iterator != this->m_UIDMap.end()) {
            return iterator->second;
        }
    }
    return MAP_UID_VALUE{};
}

void Engine::updateOrSetUIDMapValue(MAP_UID_KEY key, uint64 newDBID, anyID newClientID, std::string newClientName, uint64 newChannelID, std::string newChannelName) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
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
    }
    this->sendClientsUpdate();
}

void Engine::updateUIDMapChannelName(uint64 channelID, std::string newChannelName) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        for (auto iterator = this->m_UIDMap.begin(); iterator != this->m_UIDMap.end(); ++iterator) {
            auto value = iterator->second;
            if (value.channelID == channelID) {
                logTSMessage("Channel edited, updating name for: %s", iterator->first.c_str());
                value.channelName = newChannelName;
            }
        }
    }
    this->sendClientsUpdate();
}

void Engine::deleteUIDMapValue(MAP_UID_KEY key) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        const auto iterator = this->m_UIDMap.find(key);
        if (iterator != this->m_UIDMap.end()) {
            this->m_UIDMap.erase(iterator);
        }
    }
    this->sendClientsUpdate();
}

MAP_DBID_VALUE Engine::getDBIDMapValue(MAP_DBID_KEY key) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        const auto iterator = this->m_DBIDMap.find(key);
        if (iterator != this->m_DBIDMap.end()) {
            return iterator->second;
        }
    }
    return "";
}

void Engine::updateOrSetDBIDMapValue(MAP_DBID_KEY key, std::string newClientUID) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
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
    }
}

void Engine::deleteDBIDMapValue(MAP_DBID_KEY key) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        const auto iterator = this->m_DBIDMap.find(key);
        if (iterator != this->m_DBIDMap.end()) {
            this->m_DBIDMap.erase(iterator);
        }
    }
}

MAP_ID_VALUE Engine::getIDMapValue(MAP_ID_KEY key) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        const auto iterator = this->m_IDMap.find(key);
        if (iterator != this->m_IDMap.end()) {
            return iterator->second;
        }
    }
    return "";
}

void Engine::updateOrSetIDMapValue(MAP_ID_KEY key, std::string newClientUID) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
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
    }
}

void Engine::deleteIDMapValue(MAP_ID_KEY key) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        const auto iterator = this->m_IDMap.find(key);
        if (iterator != this->m_IDMap.end()) {
            this->m_IDMap.erase(iterator);
        }
    }
}

void Engine::addToCallbackQueue(MAP_UID_KEY key, DBID_QUEUE_MODE mode) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_DBIDCallbackQueue.emplace(key, mode);
    }
}

DBID_QUEUE_MODE Engine::getFromCallbackQueue(MAP_UID_KEY key) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        const auto iterator = this->m_DBIDCallbackQueue.find(key);
        if (iterator != this->m_DBIDCallbackQueue.end()) {
            return iterator->second;
        }
    }
    return DBID_QUEUE_MODE::UNSET;
}

void Engine::sendClientsUpdate() {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        /*std::ostringstream stringStream;
        stringStream << "{\"clients\":[";
        for (auto iterator = this->m_UIDMap.begin(); iterator != this->m_UIDMap.end(); ++iterator) {
            const auto value = iterator->second;
            if (value.clientID != UNSET_ANYID) {
                if (iterator != this->m_UIDMap.begin()) {
                    stringStream << ",";
                }
                stringStream << R"({"clientDBID":")" << value.clientDBID << R"(","clientName":")" << value.clientName << R"(","channelID":")" << value.channelID <<
                    R"(","channelName":")" << value.channelName << "\"}";
            }
        }
        stringStream << "]}";*/

		std::vector<signalr::value> clients;
        for (const auto& pair : this->m_UIDMap) {
			const auto value = pair.second;
            if (value.clientID != UNSET_ANYID) {
				std::map<std::string, signalr::value> clientMap;
				clientMap.insert(std::pair<std::string, signalr::value>("clientDBID", signalr::value(static_cast<double>(value.clientDBID))));
				clientMap.insert(std::pair<std::string, signalr::value>("clientName", value.clientName));
				clientMap.insert(std::pair<std::string, signalr::value>("channelID", signalr::value(static_cast<double>(value.channelID))));
				clientMap.insert(std::pair<std::string, signalr::value>("channelName", value.channelName));
				clients.emplace_back(clientMap);
            }
        }
        this->getSignalrClient()->sendMessage(SERVER_MESSAGE_TYPE::CLIENTS, signalr::value(clients));
    }
}

void Engine::procShutdown() {
    logTSMessage("Stopping via API");
    this->stop();
    ts3Functions.stopConnection(ts3Functions.getCurrentServerConnectionHandlerID(), "Jarvis needs to reboot");
    Sleep(2000);
    TerminateProcess(GetCurrentProcess(), 0);
}
