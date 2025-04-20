#include "lavacop.h"
#include <memory>

lavacop::lavacop() {
}

void lavacop::purge() {
	Nodes.clear();
}

void lavacop::addNode(const LavaLinkConfig &config) {
	std::lock_guard<std::mutex> lock(mutex);
	Nodes.push_back(std::make_unique<LavaLink>(config, sendPayload));
	LavaLink *newNode = Nodes.back().get();
	if (newNode->isReachable()) {
		newNode->connect();
	} else {
		std::cerr << "[lavacop:lavalink] Could not reach to the node." << std::endl;
		Nodes.pop_back();
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
	const nlohmann::json data = nlohmann::json::parse(raw);
	if (data["t"] == "VOICE_SERVER_UPDATE") {
		for (auto &nodePtr: Nodes) {
			nodePtr->handleRaw(data);
		}
	}
	if (data["t"] == "VOICE_STATE_UPDATE") {
		if (data["d"]["member"]["user"]["id"] != botId)
			return;
		if (data["d"]["channel_id"] == nullptr)
			return;
		for (auto &nodePtr: Nodes) {
			nodePtr->handleRaw(data);
		}
	}
}

LavaLink *lavacop::getIdealNode() {
	if (Nodes.empty()) {
		std::cerr << "No nodes available" << std::endl;
		return nullptr;
	}
	return Nodes[0].get();
}

Player *lavacop::getPlayer(const std::string &guildId) {
	if (Nodes.empty()) {
		throw std::runtime_error("No nodes available");
	}
	for (auto &nodePtr: Nodes) {
		try {
			return &nodePtr->getPlayer(guildId);
		} catch (std::runtime_error &e) {
			continue;
		}
	}
	throw std::runtime_error("Player not found for guildId: " + guildId);
}
