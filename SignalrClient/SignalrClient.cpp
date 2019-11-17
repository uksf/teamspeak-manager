#include "SignalrClient.h"
#include "../Common/Message.h"
#include <future>

SignalrClient::~SignalrClient() {
    this->SignalrClient::shutdown();
}

void SignalrClient::initialize(std::function<void()> connectedCallback, const std::function<void(IMessage*)> procedureCallback) {
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
		this->setClientConnecting(false);
		this->setClientConnected(false);

		this->m_connection = std::make_shared<signalr::hub_connection>("http://localhost:5000/hub/teamspeak", signalr::trace_level::errors, std::make_shared<logger>());

		this->m_connection->on("Receive", [this](const signalr::value& value) {
			const auto message = value.as_array()[0].as_string().c_str();
			logTSMessage("Received message: %s", message);
			this->m_procedureCallback(new Message(const_cast<char*>(message), std::strlen(message)));
			});

		this->setClientConnecting(true);
		this->m_connection->start([this](const std::exception_ptr exception) {
			if (exception) {
				try {
					std::rethrow_exception(exception);
				} catch (const std::exception & ex) {
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
			this->m_connection->stop([&task](const std::exception_ptr exception) {
				try {
					if (exception) {
						std::rethrow_exception(exception);
					}

					logTSMessage("Connection stopped successfully");
				} catch (const std::exception & e) {
					logTSMessage("Exception when stopping connection: %s", e.what());
				}
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
                Sleep(1000);
            } else {
                if (this->getClientConnected()) {
                    auto id = this->m_connection->get_connection_id();
                    switch (this->m_connection->get_connection_state()) {
                    case signalr::connection_state::connected:
                        IMessage* message;
						{
							std::lock_guard<std::mutex> lock(m_lockable_mutex);
							if (this->m_sendQueue.try_pop(message)) {
								const DWORD size = DWORD(message->getData().length()) + 1;
								if (size > 3) {
									this->sendMessageToClient(message->getData());
								}
								delete message;
							}
						}
                        Sleep(10);
                        break;
                    case signalr::connection_state::connecting:
                        logTSMessage("Attempting to connect to server");
                        Sleep(500);
                        break;
                    case signalr::connection_state::disconnecting:
                        logTSMessage("Attempting to disconnect from server");
                        Sleep(500);
                        break;
                    case signalr::connection_state::disconnected:
                        logTSMessage("Disconnected from server, trying to reconnect...");
                        this->connect();
                        Sleep(500);
                        break;
                    default: break;
                    }
                } else {
                    logTSMessage("Not connected, connecting...");
                    this->connect();
                    Sleep(1000);
                }
            }
        } else {
            logTSMessage("Not connected, connecting...");
            this->connect();
            Sleep(1000);
        }
    }
}

void SignalrClient::sendMessageToClient(const std::string message) const {
    const signalr::value args(message);

    if (this->m_connection) {
        this->m_connection->invoke("Invoke", args, [](const signalr::value& value, const std::exception_ptr exception) {
            if (exception) {
                try {
                    std::rethrow_exception(exception);
                } catch (const std::exception& ex) {
                    logTSMessage("Exception while sending: %s", ex.what());
                }
            }

            try {
                logTSMessage("Received: %s", value.as_string().c_str());
            } catch (const std::exception& innerException) {
                logTSMessage("Error while sending data: %s", innerException.what());
            }
        });
    }
}

void SignalrClient::sendMessage(IMessage* message) {
    if (message) {
		{
			std::lock_guard<std::mutex> lock(m_lockable_mutex);
			this->m_sendQueue.push(message);
		}
    }
}
