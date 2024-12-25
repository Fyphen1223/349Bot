#include <functional>
#include <iostream>
#include <map>
#include <vector>


#include "src/lavalink.h"

struct lavacopConfig {
	std::string botId;
	std::string userAgent = "lavacop/0.0.1";
	std::function<void(const std::string &guildId, std::string &payload)> sendPayload;
};

class lavacop {
  public:
	lavacop(const lavacopConfig &config)
		: config(config) {
		printf("lavacop created\n");
	}
	void purge() {
		Nodes.clear();
	}
	void addNode(const LavaLinkConfig &config) {
		Nodes.push_back(LavaLink(config));
		return;
	}

	/*
	TODO: Implement this function
	
	void handleRaw(const nlohmann::json &data) {
		std::string EventType = data["t"];
		if (EventType == "VOICE_SERVER_UPDATE") {
			std::string guildId = data["d"]["guild_id"];
			std::string sessionId = data["d"]["session_id"];
			std::string endpoint = data["d"]["endpoint"];
			for (auto &node: Nodes) {
				//node.sendVoiceUpdate(guildId, sessionId, endpoint);
			}
		} else if (EventType == "VOICE_STATE_UPDATE") {
			std::string guildId = data["d"]["guild_id"];
			std::string sessionId = data["d"]["session_id"];
			std::string userId = data["d"]["user_id"];
			std::string channelId = data["d"]["channel_id"];
			for (auto &node: Nodes) {
				//node.sendVoiceStateUpdate(guildId, sessionId, userId, channelId);
			}
		}
	}
	*/

  private:
	lavacopConfig config;
	std::string botId;
	std::string userAgent;
	std::vector<LavaLink> Nodes;
};