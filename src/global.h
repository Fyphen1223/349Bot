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

#endif// GLOBAL_H