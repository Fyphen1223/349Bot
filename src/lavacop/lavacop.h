#ifndef LAVACOP_H
#define LAVACOP_H

#include "src/lavalink.h"
#include <functional>
#include <iostream>
#include <map>
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
	void handleRaw(const nlohmann::json &raw);

  private:
	std::string botId;
	std::string userAgent;
	std::vector<LavaLink> Nodes;
	std::mutex mutex;
};

#endif// LAVACOP_H