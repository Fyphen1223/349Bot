#ifndef LAVALINK_H
#define LAVALINK_H

#include "hv/WebSocketClient.h"
#include "hv/requests.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <thread>
#include <utility>
#include <vector>

using namespace hv;

class WS {
  public:
	WS();

	void open(const std::string &url);
	void setPingInterval(int interval);
	void send(const std::string &msg);
	void close();
	void onMessage(const std::function<void(const std::string &)> &callback);
	void onOpen(const std::function<void()> &callback);
	void onClose(const std::function<void()> &callback);
	void removeAllMessageListeners();
	void removeAllOpenListeners();
	void removeAllCloseListeners();

  private:
	hv::WebSocketClient ws;
	hv::EventLoop loop;
	std::atomic<bool> is_open;
	reconn_setting_t reconn;
	std::vector<std::function<void(const std::string &)>> on_message_callbacks;
	std::vector<std::function<void()>> on_open_callbacks;
	std::vector<std::function<void()>> on_close_callbacks;
};

struct LavaLinkConfig {
	std::string ip;
	std::string port;
	bool secure = true;
	std::string password = "youshallnotpass";
	std::string serverName = "default";
	std::string userAgent;
	std::function<void(const std::string &guildId, std::string &payload)> sendPayload;
};

class LavaLink {
  public:
	LavaLink(const LavaLinkConfig &config);
	LavaLink(LavaLink &&other) noexcept;
	LavaLink &operator=(LavaLink &&other) noexcept;
	LavaLink(const LavaLink &) = delete;
	LavaLink &operator=(const LavaLink &) = delete;

	void open();
	void close();
	void send(const std::string &msg);
	void onReady(const std::function<void()> &callback);
	void onEvent(const std::function<void(std::string data)> &callback);
	void onState(const std::function<void(std::string data)> &callback);
	void onPlayerUpdate(const std::function<void(std::string data)> &callback);
	void onClose(const std::function<void()> &callback);
	void removeAllReadyListeners();
	void removeAllEventListeners();
	void removeAllStateListeners();
	void removeAllPlayerUpdateListeners();
	void removeAllCloseListeners();

	std::string sessionId;


  private:
	std::string url;
	WS ws;
	LavaLinkConfig config;
	std::vector<std::function<void()>> readyCallbacks;
	std::vector<std::function<void(std::string &data)>> eventsCallbacks;
	std::vector<std::function<void(std::string &data)>> stateCallbacks;
	std::vector<std::function<void(std::string &data)>> playerUpdateCallbacks;
	std::vector<std::function<void()>> closeCallbacks;
};

#endif// LAVALINK_H