#include "lavacop.h"


lavacop::lavacop() {
}

void lavacop::purge() {
	Nodes.clear();
}

void lavacop::addNode(const LavaLinkConfig &config) {
	LavaLink node(config);
	std::lock_guard<std::mutex> lock(mutex);
	Nodes.push_back(std::move(node));
	Nodes.back().connect();
}

void lavacop::setBotId(const std::string &id) {
	botId = id;
}

void lavacop::setUserAgent(const std::string &agent) {
	userAgent = agent;
}

void lavacop::handleRaw(const nlohmann::json &raw) {
	if (raw["t"] == "VOICE_SERVER_UPDATE") {
	} else if (raw["t"] == "VOICE_STATE_UPDATE") {
	}
}

LavaLink *lavacop::getIdealNode() {
	if (Nodes.empty()) {
		std::cerr << "No nodes available" << std::endl;
		return nullptr;
	}
	return &Nodes[0];
}
