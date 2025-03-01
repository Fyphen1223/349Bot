#include <dpp/dpp.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <functional>
#include <iostream>
#include <thread>

#include "events/autocomplete.h"
#include "events/commands.h"
#include "events/messageCreate.h"
#include "events/slashcommandsCreate.h"
#include "global.h"
#include "lavacop/lavacop.h"
#include "lib/log.h"
#include "lib/print.h"
#include "util/decoration.h"
#include "util/device.h"
#include "util/register.h"
#include "util/textcommands.h"
#include <hv/WebSocketClient.h>
#include <hv/requests.h>

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
	LC.setBotId(config["bot"]["applicationId"]);

	LC.setSendPayload([](const std::string &guildId, const std::string &payload) {
		BH.sendPayload(guildId, payload);
	});
	bot.on_voice_server_update([&](const dpp::voice_server_update_t &event) {
		LC.handleRawEvents(event.raw_event);
	});
	bot.on_voice_state_update([&](const dpp::voice_state_update_t &event) {
		LC.handleRawEvents(event.raw_event);
	});

	bot.on_log(DiscordLogger);
	bot.on_message_create([&](const dpp::message_create_t &event) {
		onMessageCreate(bot, event);
	});
	bot.on_slashcommand([&](const dpp::slashcommand_t &event) {
		onSlashCommands(bot, event);
	});
	bot.on_autocomplete([&](const dpp::autocomplete_t &event) {
		onAutoComplete(bot, event);
	});
	bot.on_ready([&bot, shouldRegisterSlashCommands](const dpp::ready_t &event) {
		info("Bot is ready.");

		LC.setBotId(config["bot"]["applicationId"]);
		LC.setUserAgent("LavaCop/0.0.1");
		if (shouldRegisterSlashCommands)
			registerSlashCommands(bot);
		initializeCommands();
		const std::string botId = config["bot"]["applicationId"];
		const std::vector<json> jsonNodes = config["lavalink"];

		for (const auto &node: jsonNodes) {
			LC.addNode(LavaLinkConfig{
				.ip = node["ip"].get<std::string>(),
				.port = node["port"].get<std::string>(),
				.secure = node["secure"].get<bool>(),
				.password = node["password"].get<std::string>(),
				.serverName = "default",
				.userAgent = "LavaCop/0.0.1",
				.botId = botId});
		}

		if (LC.Nodes.empty()) {
			error("No nodes available.");
			if (!config["bot"]["forceBoot"].get<bool>()) {
				bot.shutdown();
				error("No nodes available.");
				return;
			}
			info("forceBoot option is enabled.");
			info("Bot will continue to run without nodes.");
			warn("This is not recommended.");
			warn("Bot will not be able to handle music commands.");
		}
		std::thread([&bot]() {
			while (true) {
				info("----------------------------------------");
				info("Bot is running.");
				info("Ping: " + getRestPing(bot));
				info("Machine Uptime: " + getUptime());
				info("----------------------------------------");
				std::this_thread::sleep_for(std::chrono::seconds(config["log"]["reportFrequency"].get<int>()));
			}
		}).detach();
	});
	info("----------------------------------------");
	info("Device Information:");
	info("OS: " + getOS());
	info("CPU: " + getCPU());
	info("RAM: " + getRAM());
	info("Disk: " + getDisk());
	info("Kernel: " + getKernel());
	info("Uptime: " + getUptime());
	info("Hostname: " + getHostname());
	info("IP: " + getIP());
	info("----------------------------------------");

	info("Commands are being accepted.");
	std::thread([&bot]() {
		std::string input;
		while (true) {
			std::getline(std::cin, input);
			if (!input.empty()) {
				handleCLICommand(input);
			}
		}
	}).detach();

	bot.start(false);
	return 0;
}