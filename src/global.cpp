#include "global.h"

lavacop LC;

BotHandler BH;

guildQueue::guildQueue(const std::string &guildId) {
	this->guildId = guildId;
}

guildQueue::guildQueue(guildQueue &&other) noexcept {
	queue = std::move(other.queue);
	queueHistory = std::move(other.queueHistory);
	guildId = std::move(other.guildId);
	textChannelId = std::move(other.textChannelId);
	voiceChannelId = std::move(other.voiceChannelId);
	lastMessageId = std::move(other.lastMessageId);
	lastMessage = std::move(other.lastMessage);
};

guildQueue &guildQueue::operator=(guildQueue &&other) noexcept {
	if (this != &other) {
		queue = std::move(other.queue);
		guildId = std::move(other.guildId);
		textChannelId = std::move(other.textChannelId);
		voiceChannelId = std::move(other.voiceChannelId);
		lastMessageId = std::move(other.lastMessageId);
		lastMessage = std::move(other.lastMessage);
	}
	return *this;
}

Queue GQ;