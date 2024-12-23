#include <dpp/dpp.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

using json = nlohmann::json;

extern "C" {
#include <unistd.h>
}


std::ifstream rawConfig("./config.json");

json config;

int logLevel = 2;

const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

const std::string BG_RED = "\033[41m";
const std::string BG_GREEN = "\033[42m";
const std::string BG_YELLOW = "\033[43m";
const std::string BG_BLUE = "\033[44m";
const std::string BG_MAGENTA = "\033[45m";
const std::string BG_CYAN = "\033[46m";
const std::string BG_WHITE = "\033[47m";

void print(const std::string &message) {
	std::cout << message << std::endl;
	return;
}

void print(const json &object) {
	std::cout << object.dump(4) << std::endl;
	return;
}

void print(const char *message) {
	std::cout << message << std::endl;
	return;
}

std::string getCurrentTime() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
	return ss.str();
}

std::string getFormattedTime() {
	return "[" + getCurrentTime() + "]";
}

void error(const std::string &message) {
	if (4 >= logLevel)
		print(BG_RED + "[ERROR]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void warn(const std::string &message) {
	if (3 >= logLevel)
		print(BG_YELLOW + "[WARN]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void info(const std::string &message) {
	if (2 >= logLevel)
		print(BG_BLUE + "[INFO]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logDebug(const std::string &message) {
	if (1 >= logLevel)
		print(BG_GREEN + "[DEBUG]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logTrace(const std::string &message) {
	if (0 >= logLevel)
		print(BG_CYAN + "[TRACE]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logCritical(const std::string &message) {
	if (5 >= logLevel)
		print(BG_MAGENTA + "[CRITICAL]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logUnknown(const std::string &message) {
	print("[UNKNOWN]" + getFormattedTime() + ": " + message);
	return;
}

void DiscordLogger(const dpp::log_t &log) {
	std::string severity;
	switch (log.severity) {
		case dpp::ll_trace:
			severity = "TRACE";
			break;
		case dpp::ll_debug:
			severity = "DEBUG";
			break;
		case dpp::ll_info:
			severity = "INFO";
			break;
		case dpp::ll_warning:
			severity = "WARNING";
			break;
		case dpp::ll_error:
			severity = "ERROR";
			break;
		case dpp::ll_critical:
			severity = "CRITICAL";
			break;
		default:
			severity = "UNKNOWN";
			break;
	}

	if (severity == "TRACE") {
		logTrace(log.message);
	} else if (severity == "DEBUG") {
		logDebug(log.message);
	} else if (severity == "INFO") {
		info(log.message);
	} else if (severity == "WARNING") {
		warn(log.message);
	} else if (severity == "ERROR") {
		error(log.message);
	} else if (severity == "CRITICAL") {
		logCritical(log.message);
	} else {
		logUnknown(log.message);
	}
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

void onMessageCreate(const dpp::message_create_t &event) {
	if (event.msg.content.find("bad word") != std::string::npos) {
		event.reply("That is not allowed here. Please, mind your language!", true);
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

	logLevel = config["log"]["level"];
	if (logLevel < 0 || logLevel > 5) {
		logLevel = 2;
		error("Invalid log level. Defaulting to 2.");
	}

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
		if (event.msg.content.find("bad word") != std::string::npos) {
			event.reply("That is not allowed here. Please, mind your language!",
						true);
		}
	});
	bot.on_ready([&bot, shouldRegisterSlashCommands](const dpp::ready_t &event) {
		info("Bot is ready.");
		if (shouldRegisterSlashCommands)
			registerSlashCommands(bot);
	});

	bot.start(dpp::st_wait);
	return 0;
}