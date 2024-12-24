#include <dpp/dpp.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

#include "lib/log.h"
#include "lib/print.h"

using json = nlohmann::json;

extern "C" {
#include <unistd.h>
}


std::ifstream rawConfig("./config.json");

json config;

std::string getRestPing(dpp::cluster &bot) {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(bot.rest_ping));
	return std::to_string(ms.count()) + " ms";
}

void executePing(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	event.reply("Pong! REST Ping: " + getRestPing(bot));
}

int registerSlashCommands(dpp::cluster &bot) {
	dpp::slashcommand ping("ping", "Ping command", bot.me.id);
	dpp::slashcommand join("join", "Join a voice channel", bot.me.id);
	dpp::slashcommand leave("leave", "Leave the voice channel", bot.me.id);
	dpp::slashcommand play("play", "Playback things", bot.me.id);
	play.add_option(dpp::command_option(dpp::co_string, "query", "The query used for playback", true).set_auto_complete((true)));

	bot.global_bulk_command_create({ping, join, leave, play});
	info("Slash commands registered.");
	return 0;
}

bool isValidConfig(const json &data) {
	if (data.contains("bot") && data["bot"].contains("token") && data["bot"].contains("applicationId") && data.contains("log") && data["log"].contains("level") && data["log"]["level"].is_number()) {
		return true;
	}
	return false;
}

void onMessageCreate(dpp::cluster &bot, const dpp::message_create_t &event) {
	if (event.msg.content.find("bad word") != std::string::npos) {
		event.reply("That is not allowed here. Please, mind your language!", false);
	}
}

void onSlashCommands(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	const std::string commandName = event.command.get_command_name();

	if (commandName == "ping") {
		executePing(bot, event);
	}
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