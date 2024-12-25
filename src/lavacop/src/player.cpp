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


class Player {
  public:
	Player(const PlayerConfig &config)
		: config(config), lavalink(config.lavalink) {
		printf("Player created\n");
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

	void removeAllEventListeners() {
		eventsCallbacks.clear();
	}
	void removeAllStateListeners() {
		stateCallbacks.clear();
	}
	void removeAllPlayerUpdateListeners() {
		playerUpdateCallbacks.clear();
	}

  private:
	const PlayerConfig &config;
	std::vector<std::function<void(std::string &data)>> eventsCallbacks;
	std::vector<std::function<void(std::string &data)>> stateCallbacks;
	std::vector<std::function<void(std::string &data)>> playerUpdateCallbacks;
	std::string guildId;
	LavaLink *lavalink;
};