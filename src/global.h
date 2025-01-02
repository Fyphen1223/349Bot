#ifndef GLOBAL_H
#define GLOBAL_H

#include "lavacop/lavacop.h"
#include <iostream>

extern lavacop LC;

class BotHandler {
  public:
	BotHandler() {
		return;
	}

	void setBot(dpp::cluster *bot) {
		this->bot = bot;
	}

	void sendPayload(const std::string &guildId, const std::string &payload) {
		dpp::snowflake guild_id = std::stoull(guildId);
		bot->guild_get(guild_id, [this, guildId, payload](const dpp::confirmation_callback_t &event) {
			if (event.is_error())
				return;
			const auto guild = event.get<dpp::guild>();
			auto shard = bot->get_shard(guild.shard_id);
			if (shard) {
				if (shard->is_connected()) {
					shard->queue_message(payload);
				}
			}
		});
	}

  private:
	dpp::cluster *bot;
};

extern BotHandler BH;


class guildQueue {
  public:
	guildQueue() = default;
	guildQueue(const std::string &guildId);
	guildQueue(guildQueue &&other) noexcept;
	guildQueue &operator=(guildQueue &&other) noexcept;
	guildQueue(const guildQueue &) = delete;
	guildQueue &operator=(const guildQueue &) = delete;

	void add(const std::string &track) {
		queue.push_back(track);
	}

	void remove(const std::string &track) {
		queue.erase(std::remove(queue.begin(), queue.end(), track), queue.end());
	}

	void clear() {
		queue.clear();
	}

	void addHistory(const std::string &track) {
		queueHistory.push_back(track);
	}

	void removeHistory(const std::string &track) {
		queueHistory.erase(std::remove(queueHistory.begin(), queueHistory.end(), track), queueHistory.end());
	}

	void removeTrack(const int position) {
		queue.erase(queue.begin() + position);
	}

	nlohmann::json getNextTrack() {
		if (queue.empty()) {
			throw std::runtime_error("No tracks available");
		}
		return nlohmann::json::parse(queue[0]);
	}

	std::vector<std::string> queue;
	std::vector<std::string> queueHistory;
	std::string guildId;
	std::string textChannelId;
	std::string voiceChannelId;
	std::string lastMessageId;
	std::string lastMessage;

  private:
};

class Queue {
  public:
	Queue() {
		return;
	}

	void add(const std::string &guildId) {
		if (queue.find(guildId) == queue.end()) {
			queue[guildId] = (guildQueue(guildId));
		}
	}

	void remove(const std::string &guildId) {
		queue.erase(guildId);
	}

	void clear() {
		queue.clear();
	}

	std::map<std::string, guildQueue> queue;

  private:
};

extern Queue GQ;

#endif// GLOBAL_H