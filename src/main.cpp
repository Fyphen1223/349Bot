#include <dpp/dpp.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

#include "events/messageCreate.h"
#include "events/slashcommandsCreate.h"
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
					 dpp::i_default_intents | dpp::i_message_content);

	bot.on_log(DiscordLogger);
	bot.on_message_create([&bot](const dpp::message_create_t &event) {
		onMessageCreate(bot, event);
	});
	bot.on_slashcommand([&bot](const dpp::slashcommand_t &event) {
		onSlashCommands(bot, event);
	});
	bot.on_ready([&bot, shouldRegisterSlashCommands](const dpp::ready_t &event) {
		info("Bot is ready.");
		if (shouldRegisterSlashCommands)
			registerSlashCommands(bot);
	});

	bot.start(dpp::st_wait);
	return 0;
}