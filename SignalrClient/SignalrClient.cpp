#include "SignalrClient.h"
#include "../Common/ClientMessage.h"
#include <future>

SignalrClient::~SignalrClient() {
    this->SignalrClient::shutdown();
}

void SignalrClient::initialize(const std::function<void()> connectedCallback, const std::function<void(ClientMessage)> procedureCallback) {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->SignalrClient::setClientConnecting(false);
        this->SignalrClient::setClientConnected(false);
        this->SignalrClient::setShuttingDown(false);
        this->m_connectedCallback = connectedCallback;
        this->m_procedureCallback = procedureCallback;
        this->m_workerThread = std::thread(&SignalrClient::workerLoop, this);
    }
}

void SignalrClient::connect() {
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_connection = nullptr;
        this->setClientConnected(false);

        this->m_connection = std::make_shared<signalr::hub_connection>("http://localhost:5000/hub/teamspeak", signalr::trace_level::errors, std::make_shared<logger>());

        this->m_connection->on("Receive", [this](const signalr::value& value) {
            const auto procedure = static_cast<CLIENT_MESSAGE_TYPE>(static_cast<int>(value.as_array()[0].as_double()));
            const std::map<std::string, signalr::value> args = value.as_array()[1].as_map();
            this->m_procedureCallback(ClientMessage(procedure, args));
        });

        this->setClientConnecting(true);
        this->m_connection->start([this](const std::exception_ptr exception) {
            if (exception) {
                try {
                    std::rethrow_exception(exception);
                } catch (const std::exception& ex) {
                    logTSMessage("Exception when starting connection: %s", ex.what());
                    this->setClientConnected(false);
                    this->setClientConnecting(false);
                }
            } else {
                logTSMessage("Connected");
                this->setClientConnected(true);
                this->setClientConnecting(false);
                this->m_connectedCallback();
            }
        });
    }
}

void SignalrClient::shutdown() {
    if (this->getShuttingDown()) return;
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->setShuttingDown(true);
        if (this->m_workerThread.joinable()) {
            this->m_workerThread.join();
        }
        this->m_sendQueue.clear();

        std::promise<void> task;
        if (this->m_connection && this->m_connection->get_connection_state() != signalr::connection_state::disconnected) {
            this->m_connection->stop([this, &task](const std::exception_ptr exception) {
                try {
                    if (exception) {
                        std::rethrow_exception(exception);
                    }

                    logTSMessage("Connection stopped successfully");
                } catch (const std::exception& e) {
                    logTSMessage("Exception when stopping connection: %s", e.what());
                }
                this->m_connection = nullptr;
                task.set_value();
            });
        } else {
            task.set_value();
        }
        task.get_future().get();
        this->setClientConnecting(false);
        this->setClientConnected(false);
        this->setShuttingDown(false);
    }
}


void SignalrClient::workerLoop() {
    while (!this->getShuttingDown()) {
        if (this->m_connection) {
            if (this->getClientConnecting()) {
                logTSMessage("Waiting for connection...");
                wait(10000, [this]() {
                    return !this->getClientConnecting();
                });
            } else {
                if (this->getClientConnected()) {
                    auto id = this->m_connection->get_connection_id();
                    switch (this->m_connection->get_connection_state()) {
                    case signalr::connection_state::connected: {
                        std::lock_guard<std::mutex> lock(m_lockable_mutex);
                        std::pair<SERVER_MESSAGE_TYPE, signalr::value> message;
                        if (this->m_sendQueue.try_pop(message)) {
                            if (message.first != SERVER_MESSAGE_TYPE::EMPTY_EVENT) {
                                this->sendMessageToClient(message);
                            }
                        }
                    }
						this->wait(5);
                        break;
                    case signalr::connection_state::connecting:
                        logTSMessage("Attempting to connect to server");
						this->wait(500);
                        break;
                    case signalr::connection_state::disconnecting:
                        logTSMessage("Attempting to disconnect from server");
						this->wait(500);
                        break;
                    case signalr::connection_state::disconnected:
                        logTSMessage("Disconnected from server, trying to reconnect...");
                        this->connect();
                        this->wait(500);
                        break;
                    default: break;
                    }
                } else {
                    logTSMessage("Not connected, connecting...");
                    this->connect();
                }
            }
        } else {
            logTSMessage("Not connected, connecting...");
            this->connect();
        }
    }
}

void SignalrClient::wait(int duration, const std::function<bool()> predicate) {
    while (duration != 0) {
		if (this->getShuttingDown() || predicate()) break;
        Sleep(1);
        duration--;
    }
}


void SignalrClient::sendMessageToClient(const std::pair<SERVER_MESSAGE_TYPE, signalr::value> message) const {
    const std::vector<signalr::value> arr{static_cast<double>(message.first), message.second};
    const signalr::value args(arr);

    if (this->m_connection) {
        this->m_connection->invoke("Invoke", args, [](const signalr::value& value, std::exception_ptr exception) {
            try {
                if (exception) {
                    std::rethrow_exception(exception);
                }

                if (value.is_string()) {
                    logTSMessage("Invoke returned: %s", value.as_string().c_str());
                }
            } catch (const std::exception& e) {
                logTSMessage("Error while sending data: %s", e.what());
            }
        });
    }
}

void SignalrClient::sendMessage(SERVER_MESSAGE_TYPE procedure, signalr::value value) {
    if (procedure == SERVER_MESSAGE_TYPE::EMPTY_EVENT) return;
    {
        std::lock_guard<std::mutex> lock(m_lockable_mutex);
        this->m_sendQueue.push(std::pair<SERVER_MESSAGE_TYPE, signalr::value>(procedure, value));
    }
}
