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

//#include <../lavacop.h>
using namespace hv;

struct PlayerConfig {
	std::function<void(const std::string &guildId, std::string &payload)> sendPayload;
};


class Player {
  public:
	Player(const PlayerConfig &config)
		: config(config) {
	}

	void onReady(const std::function<void()> &callback) {
		readyCallbacks.push_back(callback);
	}
	void onEvent(const std::function<void(std::string data)> &callback) {
		eventsCallbacks.push_back(callback);
	}
	void onState(const std::function<void(std::string data)> &callback) {
		stateCallbacks.push_back(callback);
	}
	void onPlayerUpdate(const std::function<void(std::string data)> &callback) {
		playerUpdateCallbacks.push_back(callback);
	}
	void onClose(const std::function<void()> &callback) {
		closeCallbacks.push_back(callback);
	}

	void removeAllReadyListeners() {
		readyCallbacks.clear();
	}
	void removeAllEventListeners() {
		eventsCallbacks.clear();
	}
	void removeAllStateListeners() {
		stateCallbacks.clear();
	}
	void removeAllPlayerUpdateListeners() {
		playerUpdateCallbacks.clear();
	}
	void removeAllCloseListeners() {
		closeCallbacks.clear();
	}


  private:
	PlayerConfig config;
	std::vector<std::function<void()>> readyCallbacks;
	std::vector<std::function<void(std::string &data)>> eventsCallbacks;
	std::vector<std::function<void(std::string &data)>> stateCallbacks;
	std::vector<std::function<void(std::string &data)>> playerUpdateCallbacks;
	std::vector<std::function<void()>> closeCallbacks;
	std::string guildId;
	std::string fetchUrl;
	LavaLink lavalink;
};