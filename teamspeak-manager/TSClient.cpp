#include "TSClient.h"
#include "Engine.h"

extern TS3Functions ts3Functions;

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

void TSClient::updateServerGroups(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));

    ts3Functions.requestServerGroupsByClientID(ts3Functions.getCurrentServerConnectionHandlerID(), clientDBID, nullptr);
}

void TSClient::assignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    ts3Functions.requestServerGroupAddClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr);
}

void TSClient::unassignServerGroup(std::vector<std::string> args) {
    const uint64 clientDBID = std::stoul(args.at(0));
    const uint64 serverGroupID = std::stoul(args.at(1));

    ts3Functions.requestServerGroupDelClient(ts3Functions.getCurrentServerConnectionHandlerID(), serverGroupID, clientDBID, nullptr);
}

