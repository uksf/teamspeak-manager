#include "SignalrClient.h"
#include "../Common/ClientMessage.h"
#include <future>
#include "../Manager/Engine.h"
#include "../Manager/Data.h"

void SignalrClient::initialize() {
	logTSMessage("Signalr: Initialising");
	this->setNewState(CONNECTION_STATE::DISCONNECTED);
	this->m_disconnectRequested = false;

	this->m_connection = std::make_shared<signalr::hub_connection>("http://localhost:5000/hub/teamspeak", signalr::trace_level::errors, std::make_shared<logger>());

	this->m_connection->on("Receive", [](const signalr::value& value) {
		logTSMessage("Signalr: received message tid: %u", std::this_thread::get_id());
		const auto procedure = static_cast<CLIENT_MESSAGE_TYPE>(static_cast<int>(value.as_array()[0].as_double()));
		const std::map<std::string, signalr::value> args = value.as_array()[1].as_map();
		Engine::getInstance()->addToReceiveQueue(ClientMessage(procedure, args));
	});

	logTSMessage("Signalr: Initialization complete");
}

void SignalrClient::requestDisconnect() {
	logTSMessage("Signalr: Disconnect requested");
	this->m_disconnectRequested = true;
}

void SignalrClient::connect() {
	if (this->getState() == CONNECTION_STATE::DISCONNECTING) return;
	logTSMessage("Signalr: Connecting");
	this->setNewState(CONNECTION_STATE::CONNECTING);

	if (this->m_connection == nullptr) {
		logTSMessage("Signalr: Trying to connect but connection object is null");
	}

	this->m_connection->start([this](const std::exception_ptr exception) {
		if (exception) {
			try {
				std::rethrow_exception(exception);
			} catch (const std::exception & ex) {
				logTSMessage("Signalr: Failed to connect: %s", ex.what());
				this->setNewState(CONNECTION_STATE::DISCONNECTED);
			}
		} else {
			logTSMessage("Signalr: Connected");
			this->setNewState(CONNECTION_STATE::CONNECTED);
			Engine::getInstance()->addToFunctionQueue([]() {
				Data::getInstance()->populateClientMaps();
			});
		}
	});
}

void SignalrClient::updateConnectionState() {
	if (this->m_connection == nullptr) return;
    if (this->m_connection->get_connection_state() == signalr::connection_state::disconnected) {
		this->setNewState(CONNECTION_STATE::DISCONNECTED);
    }
}

void SignalrClient::disconnect() {
	if (this->getState() == CONNECTION_STATE::DISCONNECTED || this->getState() == CONNECTION_STATE::DISCONNECTING) return;
	logTSMessage("Signalr: Disconnecting");
	this->setNewState(CONNECTION_STATE::DISCONNECTING);

	std::promise<void> task;
	if (this->m_connection && this->m_connection->get_connection_state() != signalr::connection_state::disconnected) {
		this->m_connection->stop([this, &task](const std::exception_ptr exception) {
			try {
				if (exception) {
					std::rethrow_exception(exception);
				}

				logTSMessage("Signalr: Disconnected");
			} catch (const std::exception & e) {
				logTSMessage("Signalr: Failed to disconnect: %s", e.what());
			}
			this->m_connection = nullptr;
			task.set_value();
		});
	} else {
		logTSMessage("Signalr: Already disconnected");
		this->m_connection = nullptr;
		task.set_value();
	}
	task.get_future().get();
	this->setNewState(CONNECTION_STATE::DISCONNECTED);
}

CONNECTION_STATE SignalrClient::getState() const {
	return this->m_state;
}

bool SignalrClient::isDisconnectRequested() const {
	return this->m_disconnectRequested;
}

void SignalrClient::setNewState(const CONNECTION_STATE state) {
	{
		std::lock_guard<std::mutex> lock(m_lockable_mutex);
		this->m_state = state;
	}
}

void SignalrClient::sendMessage(std::pair<SERVER_MESSAGE_TYPE, signalr::value> message) const {
	if (message.first == SERVER_MESSAGE_TYPE::EMPTY_EVENT) return;
	logTSMessage("Signalr: Send message tid: %u", std::this_thread::get_id());
	const std::vector<signalr::value> arr{ static_cast<double>(message.first), message.second };
	const signalr::value args(arr);

	if (this->m_connection) {
		this->m_connection->invoke("Invoke", args, [](const signalr::value& value, std::exception_ptr exception) {
			try {
				if (exception) {
					std::rethrow_exception(exception);
				}

				if (value.is_string()) {
					logTSMessage("Signalr: Invoke returned: %s", value.as_string().c_str());
				}
			} catch (const std::exception & e) {
				logTSMessage("Signalr: Error while sending data: %s", e.what());
			}
		});
	}
}
