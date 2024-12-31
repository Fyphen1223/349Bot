#ifndef LAVALINK_H
#define LAVALINK_H

#include <atomic>
#include <chrono>
#include <functional>
#include <hv/WebSocketClient.h>
#include <hv/requests.h>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class WS;
class LavaLink;
class Player;

struct LavaLinkConfig {
	std::string ip;
	std::string port;
	bool secure = true;
	std::string password = "youshallnotpass";
	std::string serverName = "default";
	std::string userAgent;
	std::string botId;
};

struct PlayerConfig {
	std::function<void(const std::string &guildId, const std::string &payload)> sendPayload;
	LavaLink *lavalink;
	std::string guildId;
};

struct connectionInfo {
	std::string token;
	std::string endpoint;
	std::string sessionId;
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

  private:
	reconn_setting_t reconn;
	std::vector<std::function<void(const std::string &)>> on_message_callbacks;
	std::vector<std::function<void()>> on_open_callbacks;
	std::vector<std::function<void()>> on_close_callbacks;
};

class LavaLink {
  public:
	LavaLink() = default;
	LavaLink(const LavaLinkConfig &config, const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload);
	LavaLink(LavaLink &&other) noexcept;
	LavaLink &operator=(LavaLink &&other) noexcept;
	LavaLink(const LavaLink &) = delete;
	LavaLink &operator=(const LavaLink &) = delete;

	void close();
	void send(const std::string &msg);
	void connect();
	bool isReachable();

	void onReady(const std::function<void()> &callback);
	void onClose(const std::function<void()> &callback);
	void onMessage(const std::function<void(std::string data)> &callback);

	void removeAllReadyListeners();
	void removeAllCloseListeners();
	void removeAllMessageListeners();

	void emitReady();
	void emitClose();
	void emitMessage(const std::string &msg);

	void setSendPayload(const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload);

	nlohmann::json loadTracks(const std::string &identifier);

	void join(const std::string &guildId, const std::string &channelId, const bool &selfDeaf = false, const bool &selfMute = false);
	void leave(const std::string &guildId);

	void handleRaw(const nlohmann::json &raw);

	Player &getPlayer(const std::string &guildId);

	std::string url;
	std::string fetchUrl;
	std::string password;
	std::string botId;
	std::string userAgent;
	LavaLinkConfig config;

	std::map<std::string, Player> Players;

	std::string sessionId;

  private:
	WS ws;
	std::vector<std::function<void()>> readyCallbacks;
	std::vector<std::function<void()>> closeCallbacks;
	std::vector<std::function<void(std::string data)>> messageCallbacks;


	std::function<void(const std::string &guildId, const std::string &payload)> sendPayload;
};

class Player {
  public:
	Player() = default;
	Player(Player &&other) noexcept;
	Player &operator=(Player &&other) noexcept;
	Player(const Player &) = delete;
	Player &operator=(const Player &) = delete;
	Player(const PlayerConfig &config);

	void onState(const std::function<void(std::string data)> &callback) {
		stateCallbacks.push_back(callback);
	}
	void onPlayerUpdate(const std::function<void(std::string data)> &callback) {
		playerUpdateCallbacks.push_back(callback);
	}
	void onTrackStart(const std::function<void(std::string data)> &callback) {
		trackStartCallbacks.push_back(callback);
	}
	void onTrackEnd(const std::function<void(std::string data)> &callback) {
		trackEndCallbacks.push_back(callback);
	}
	void onTrackException(const std::function<void(std::string data)> &callback) {
		trackExceptionCallbacks.push_back(callback);
	}
	void onTrackStuck(const std::function<void(std::string data)> &callback) {
		trackStuckCallbacks.push_back(callback);
	}
	void onWebSocketClosed(const std::function<void(std::string data)> &callback) {
		webSocketClosedCallbacks.push_back(callback);
	}

	void removeAllStateListeners() {
		stateCallbacks.clear();
	}
	void removeAllPlayerUpdateListeners() {
		playerUpdateCallbacks.clear();
	}
	void removeAllTrackStartListeners() {
		trackStartCallbacks.clear();
	}
	void removeAllTrackEndListeners() {
		trackEndCallbacks.clear();
	}
	void removeAllTrackExceptionListeners() {
		trackExceptionCallbacks.clear();
	}

	void handleEvents(const nlohmann::json &raw);
	void handleLavaLinkEvents(std::string data);
	void connect();
	void get();

	nlohmann::json update(const nlohmann::json &data, const bool noReplace = false);

	void play(const std::string &track, const int startTime = 0, const int endTime = 0, const bool noReplace = false);
	void pause();
	void resume();
	void stop();
	void seek(const int position);
	void volume(const int volume);
	//void destroy();

	bool paused = false;
	bool playing = false;
	bool stuck = false;
	bool ended = false;
	bool closed = false;

	bool connected = false;

	int position = 0;
	int volumeLevel = 100;
	int ping = 0;


  private:
	PlayerConfig config;
	LavaLink *Node;
	std::string guildId;

	std::vector<std::function<void(std::string data)>> stateCallbacks;
	std::vector<std::function<void(std::string data)>> playerUpdateCallbacks;
	std::vector<std::function<void(std::string data)>> trackStartCallbacks;
	std::vector<std::function<void(std::string data)>> trackEndCallbacks;
	std::vector<std::function<void(std::string data)>> trackExceptionCallbacks;
	std::vector<std::function<void(std::string data)>> trackStuckCallbacks;
	std::vector<std::function<void(std::string data)>> webSocketClosedCallbacks;

	std::function<void(const std::string &guildId, const std::string &payload)> sendPayload;

	std::string token;
	std::string sessionId;
	std::string endpoint;
};


#endif// LAVALINK_H