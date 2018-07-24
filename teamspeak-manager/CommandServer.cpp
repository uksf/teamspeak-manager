#include "CommandServer.h"

#include "ts3_functions.h"
#include "TextMessage.h"
#include "Log.h"
#include "Engine.h"
#include "teamspeak/public_rare_definitions.h"

CommandServer::CommandServer() {
    this->CommandServer::setCommandId(nullptr);
    this->CommandServer::setConnected(TRUE);
}

CommandServer::CommandServer(const ID id) {
    this->CommandServer::setId(id);
}

CommandServer::~CommandServer() = default;

void CommandServer::initialize(){
    TRACE("enter");
}

void CommandServer::shutdown() {
    TRACE("enter");
}

void CommandServer::sendMessage(IMessage *msg){
    LOCK(this);
    ts3Functions.sendPluginCommand(static_cast<unsigned __int64>(ts3Functions.getCurrentServerConnectionHandlerID()),
        static_cast<const char *>(this->getCommandId()), 
        reinterpret_cast<const char *>(msg->getData()), 
        PluginCommandTarget_CURRENT_CHANNEL, nullptr, nullptr);

    delete msg;
    UNLOCK(this);
}

void CommandServer::handleMessage(unsigned char *data){
    auto*msg = new TextMessage(reinterpret_cast<char *>(data), strlen(reinterpret_cast<char *>(data)));
    if (Engine::getInstance()->getProcedureEngine() && msg) {
        Engine::getInstance()->getProcedureEngine()->runProcedure(static_cast<IServer *>(this), static_cast<IMessage *>(msg));
    }
}

void CommandServer::release() {
    if (this->getCommandId()) {
        free(this->getCommandId());
    }
}