#include <player.h>

#include "hv/WebSocketClient.h"
#include "hv/requests.h"
#include "lavalink.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <thread>
#include <utility>
#include <vector>

using namespace hv;

struct PlayerConfig {
	std::function<void(const std::string &guildId, std::string &payload)> sendPayload;
	LavaLink *lavalink;
};

struct connectionInfo {
	std::string token;
	std::string endpoint;
	std::string sessionId;
};



class Player {
  public:
	Player(const PlayerConfig &config)
		: config(config), lavalink(config.lavalink) {
		printf("Player created\n");
	}

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
	void removeAllTrackStuckListeners() {
		trackStuckCallbacks.clear();
	}
	void removeAllWebSocketClosedListeners() {
		webSocketClosedCallbacks.clear();
	}

	void emitTrackStart(std::string &data) {
		for (auto &callback: trackStartCallbacks) {
			callback(data);
		}
	};
	void emitTrackEnd(std::string &data) {
		for (auto &callback: trackEndCallbacks) {
			callback(data);
		}
	};
	void emitTrackException(std::string &data) {
		for (auto &callback: trackExceptionCallbacks) {
			callback(data);
		}
	};
	void emitTrackStuck(std::string &data) {
		for (auto &callback: trackStuckCallbacks) {
			callback(data);
		}
	};
	void emitWebSocketClosed(std::string &data) {
		for (auto &callback: webSocketClosedCallbacks) {
			callback(data);
		}
	};

	int position = 0;
	int volume = 100;
	bool paused = false;
	bool playing = false;

	std::string guildId;
	LavaLink *lavalink;

  private:
	const PlayerConfig &config;
	std::vector<std::function<void(std::string &data)>> stateCallbacks;
	std::vector<std::function<void(std::string &data)>> playerUpdateCallbacks;
	std::vector<std::function<void(std::string &data)>> trackStartCallbacks;
	std::vector<std::function<void(std::string &data)>> trackEndCallbacks;
	std::vector<std::function<void(std::string &data)>> trackExceptionCallbacks;
	std::vector<std::function<void(std::string &data)>> trackStuckCallbacks;
	std::vector<std::function<void(std::string &data)>> webSocketClosedCallbacks;
};