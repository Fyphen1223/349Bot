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
		dpp::guild *guild = dpp::find_guild(guild_id);
		if (guild) {
			auto shard = bot->get_shard(guild->shard_id);
			if (shard) {
				shard->socket_write(payload);
			}
		}
	}

  private:
	dpp::cluster *bot;
};


extern BotHandler BH;

#endif// GLOBAL_H