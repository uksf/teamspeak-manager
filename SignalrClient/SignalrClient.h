#pragma once

#include "SignalrCommon.h"
#include "../Common/Lockable.h"
#include <concurrent_queue.h>
#include <signalrclient/log_writer.h>
#include <signalrclient/hub_connection.h>
#include "../Common/Singleton.h"

class logger final : public signalr::log_writer {
	void __cdecl write(const std::string& entry) override {
		logTSMessage(entry.c_str());
	}
};

class SignalrClient final : public Singleton<SignalrClient>, public Lockable {
public:
	SignalrClient() = default;
	~SignalrClient() = default;

	void initialize();
	void disconnect();
	void requestDisconnect();
	void connect();
	void updateConnectionState();

    [[nodiscard]] CONNECTION_STATE getState() const;
    [[nodiscard]] bool isDisconnectRequested() const;

	void sendMessage(std::pair<SERVER_MESSAGE_TYPE, signalr::value> message) const;

private:
	std::shared_ptr<signalr::hub_connection> m_connection;
	CONNECTION_STATE m_state = CONNECTION_STATE::DISCONNECTED;
	bool m_disconnectRequested{};

	void setNewState(CONNECTION_STATE state);
};
