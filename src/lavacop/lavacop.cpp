#include "lavacop.h"


lavacop::lavacop() {
}

void lavacop::purge() {
	Nodes.clear();
}

void lavacop::addNode(const LavaLinkConfig &config) {
	LavaLink node(config, sendPayload);
	std::lock_guard<std::mutex> lock(mutex);
	if (node.isReachable()) {
		Nodes.push_back(std::move(node));
		Nodes.back().connect();
	} else {
		std::cerr << "[lavacop:lavalink] Could not reach to the node." << std::endl;
	}
}

void lavacop::setBotId(const std::string &id) {
	botId = id;
}

void lavacop::setUserAgent(const std::string &agent) {
	userAgent = agent;
}

void lavacop::setSendPayload(const std::function<void(const std::string &guildId, const std::string &payload)> &sendPayload) {
	this->sendPayload = sendPayload;
};

void lavacop::handleRawEvents(const std::string &raw) {
	printf("Raw event: %s\n", raw.c_str());
}

LavaLink *lavacop::getIdealNode() {
	if (Nodes.empty()) {
		std::cerr << "No nodes available" << std::endl;
		return nullptr;
	}
	return &Nodes[0];
}