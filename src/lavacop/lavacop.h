#ifndef LAVACOP_H
#define LAVACOP_H

#include "src/lavalink.h"
#include <dpp/dpp.h>
#include <functional>
#include <iostream>
#include <map>
#include <memory>// Include for std::unique_ptr
#include <mutex>
#include <nlohmann/json.hpp>
#include <vector>

class lavacop {
  public:
	lavacop();
	void purge();
	void addNode(const LavaLinkConfig &config);
	void setBotId(const std::string &id);
	void setUserAgent(const std::string &agent);
	LavaLink *getIdealNode();
	void handleRawEvents(const std::string &raw);
	void setSendPayload(const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload);
	Player *getPlayer(const std::string &guildId);

	std::vector<std::unique_ptr<LavaLink>> Nodes;// Use unique_ptr

  private:
	std::string botId;
	std::string userAgent;
	std::mutex mutex;
	std::function<void(const std::string &guildId, const std::string &payload)> sendPayload;
};

#endif// LAVACOP_H
