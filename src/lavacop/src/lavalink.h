#ifndef LAVALINK_H
#define LAVALINK_H

#include "../../lib/log.h"
#include "hv/WebSocketClient.h"
#include "hv/requests.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <vector>

struct LavaLinkConfig {
	std::string ip;
	std::string port;
	bool secure = true;
	std::string password = "youshallnotpass";
	std::string serverName = "default";
	std::string userAgent;
	std::function<void(const std::string &guildId, std::string &payload)> sendPayload;
	std::string botId;
};

class WS {
  public:
	WS();
	void open(const std::string &url, const http_headers &headers = {});
	void setPingInterval(int interval);
	void send(const std::string &msg);
	void close();
	void onMessage(const std::function<void(const std::string &)> &callback);
	void onOpen(const std::function<void()> &callback);
	void onClose(const std::function<void()> &callback);
	void removeAllMessageListeners();
	void removeAllOpenListeners();
	void removeAllCloseListeners();
	bool isOpen() {
		return is_open;
	}

	hv::WebSocketClient ws;
	std::atomic<bool> is_open;
	//hv::EventLoop loop;

  private:
	reconn_setting_t reconn;
	std::vector<std::function<void(const std::string &)>> on_message_callbacks;
	std::vector<std::function<void()>> on_open_callbacks;
	std::vector<std::function<void()>> on_close_callbacks;
};

class LavaLink {
  public:
	LavaLink() = default;
	LavaLink(const LavaLinkConfig &config);
	LavaLink(LavaLink &&other) noexcept;
	LavaLink &operator=(LavaLink &&other) noexcept;
	LavaLink(const LavaLink &) = delete;
	LavaLink &operator=(const LavaLink &) = delete;

	void close();
	void send(const std::string &msg);
	void connect();

	void onReady(const std::function<void()> &callback);
	void onState(const std::function<void(std::string data)> &callback);
	void onPlayerUpdate(const std::function<void(std::string data)> &callback);
	void onClose(const std::function<void()> &callback);

	void removeAllReadyListeners();
	void removeAllStateListeners();
	void removeAllPlayerUpdateListeners();
	void removeAllCloseListeners();

	void emitReady();
	void emitState(std::string &data);
	void emitPlayerUpdate(std::string &data);
	void emitClose();

	nlohmann::json loadTracks(const std::string &identifier);

	std::string url;
	std::string fetchUrl;
	std::string password;
	std::string botId;
	std::string userAgent;
	LavaLinkConfig config;

  private:
	WS ws;
	std::vector<std::function<void()>> readyCallbacks;
	std::vector<std::function<void(std::string &data)>> stateCallbacks;
	std::vector<std::function<void(std::string &data)>> playerUpdateCallbacks;
	std::vector<std::function<void()>> closeCallbacks;
};

#endif// LAVALINK_H