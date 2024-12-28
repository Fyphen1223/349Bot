#include <dpp/dpp.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <functional>
#include <iostream>
#include <thread>

#include "events/messageCreate.h"
#include "events/slashcommandsCreate.h"
#include "global.h"
#include "lavacop/lavacop.h"
#include "lib/log.h"
#include "lib/print.h"
#include "util/register.h"

using json = nlohmann::json;

std::ifstream rawConfig("./config.json");

json config;


bool isValidConfig(const json &data) {
	if (data.contains("bot") && data["bot"].contains("token") && data["bot"].contains("applicationId") && data.contains("log") && data["log"].contains("level") && data["log"]["level"].is_number()) {
		return true;
	}
	return false;
}

int main(int argc, char *argv[]) {
	if (rawConfig.is_open()) {
		info("Config file opened successfully.");
	} else {
		error("Failed to parse config file.");
		return 1;
	}

	try {
		config = json::parse(rawConfig);
	} catch (const json::parse_error &e) {
		error("Illegal JSON format in config.json.");
		return 1;
	}

	if (!isValidConfig(config)) {
		error("Invalid config file.");
		return 1;
	}
	info("Valid config file.");
	int configLogLevel = config["log"]["level"];
	if (configLogLevel < 0 || configLogLevel > 5) {
		setLogLevel(2);
		error("Invalid log level. Defaulting to 2.");
	}
	setLogLevel(configLogLevel);

	bool shouldRegisterSlashCommands = false;
	for (int i = 0; i < argc; ++i) {
		if (std::string(argv[i]) == "--help") {
			info("Help is available at this project's GitHub page.");
			info("Exiting.");
			return 0;
		}
		if (std::string(argv[i]) == "--version") {
			info("Version 0.0.1");
		}
		if (std::string(argv[i]) == "--register-slashcommands") {
			shouldRegisterSlashCommands = true;
		}
	}

	if (getuid() == 0)
		warn("Running as root. This is not recommended.");

	dpp::cluster bot(config["bot"]["token"],
					 dpp::intents::i_all_intents);

	BH.setBot(&bot);

	LC.setSendPayload([](const std::string &guildId, const std::string &payload) {
		BH.sendPayload(guildId, payload);
	});
	bot.on_voice_state_update([&](const dpp::voice_state_update_t &event) {
		const nlohmann::json raw = json::parse(event.raw_event);
		LC.handleRaw(raw);
	});
	bot.on_voice_server_update([&](const dpp::voice_server_update_t &event) {
		const nlohmann::json raw = json::parse(event.raw_event);
		LC.handleRaw(raw);
	});

	bot.on_log(DiscordLogger);
	bot.on_message_create([&](const dpp::message_create_t &event) {
		onMessageCreate(bot, event);
	});
	bot.on_slashcommand([&](const dpp::slashcommand_t &event) {
		onSlashCommands(bot, event);
	});
	bot.on_ready([&bot, shouldRegisterSlashCommands](const dpp::ready_t &event) {
		info("Bot is ready.");
		LC.setBotId(config["bot"]["applicationId"]);
		LC.setUserAgent("LavaCop/0.0.1");
		if (shouldRegisterSlashCommands)
			registerSlashCommands(bot);

		const std::string botId = config["bot"]["applicationId"];
		const std::function<void(const std::string &, std::string &)> sendPayload = [](const std::string &guildId, std::string &payload) {
			print("Payload sent to guild " + guildId + ": " + payload);
		};
		LC.addNode(LavaLinkConfig{.ip = "localhost", .port = "2333", .secure = false, .password = "youshallnotpass", .serverName = "default", .userAgent = "LavaCop/0.0.1", .botId = botId});

		std::this_thread::sleep_for(std::chrono::seconds(5));
		const std::string guild_id_str = "919809544648020008";
		dpp::snowflake guild_id = static_cast<dpp::snowflake>(std::stoull(guild_id_str));
		const std::string channel_id_str = "919809544648020012";
		dpp::snowflake channel_id = static_cast<dpp::snowflake>(std::stoull(channel_id_str));
		LC.getIdealNode()->join(guild_id, channel_id, true, true);
	});
	//bot.get_rest()
	bot.start(dpp::st_wait);
	return 0;
}