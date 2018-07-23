#include "TSClient.h"
#include "Engine.h"

void TSClient::start() {
    Engine::getInstance()->start();
    this->setState(STATE_RUNNING);
}

void TSClient::stop() {
    if (Engine::getInstance() != nullptr) {
        Engine::getInstance()->stop();
        this->setState(STATE_STOPPING);
        this->setState(STATE_STOPPED);
    }
}

void TSClient::assignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(1));
    const uint64 serverGroupID = std::stoul(args.at(2));

    const uint64 serverConnectionHandleID = ts3Functions.getCurrentServerConnectionHandlerID();
    ts3Functions.requestServerGroupAddClient(serverConnectionHandleID, serverGroupID, clientDBID, nullptr);
}

void TSClient::unassignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(1));
    const uint64 serverGroupID = std::stoul(args.at(2));

    const uint64 serverConnectionHandleID = ts3Functions.getCurrentServerConnectionHandlerID();
    ts3Functions.requestServerGroupDelClient(serverConnectionHandleID, serverGroupID, clientDBID, nullptr);
}

