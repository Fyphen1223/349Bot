#ifndef LAVALINK_H
#define LAVALINK_H

#include "hv/WebSocketClient.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <vector>

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

class LavaLink {
  public:
	LavaLink(const std::string &url);
	void open();
	void close();
	void send(const std::string &msg);

  private:
	std::string url;
	WS ws;
};

#endif// LAVALINK_H