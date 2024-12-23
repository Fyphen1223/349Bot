#include <dpp/dpp.h>
#include <fstream>
#include <nlohmann/json.hpp>

#include <iostream>

using json = nlohmann::json;

extern "C" {
#include <unistd.h>
}


std::ifstream rawConfig("./config.json");

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

void error(const std::string &message) {
	print(BG_RED + "[ERR]: " + message + RESET);
	return;
}

void warn(const std::string &message) {
	print(BG_YELLOW + "[WARN]" + RESET + ": " + message + RESET);
	return;
}

void info(const std::string &message) {
	print(BG_BLUE + "[INFO]" + RESET + ": " + message);
	return;
}

bool isValidConfig(const json &data) {
	if (data.contains("bot") && data["bot"].contains("token") && data["bot"].contains("applicationId")) {
		return true;
	}
	return false;
}

int main() {
	if (rawConfig.is_open()) {
		info("Config file opened successfully.");
	} else {
		error("Failed to parse config file.");
		return 1;
	}

	json config;
	try {
		config = json::parse(rawConfig);
	} catch (const json::parse_error &e) {
		error("Illegal JSON format in config.json.");
		return 1;
	}

	print(isValidConfig(config) ? "Valid config file." : "Invalid config file.");

	/*
	dpp::cluster bot(
		"MTI4MjUyMTY2NzQ1Njg2NDI5Ng.GC6b6V.jhNACAu1nZye5_qGxVieuC_"
		"Rjqn7VtNE7bUhGs",
		dpp::i_default_intents | dpp::i_message_content);

	bot.on_log(dpp::utility::cout_logger());
	bot.on_message_create([&bot](const dpp::message_create_t &event) {
		if (event.msg.content.find("bad word") != std::string::npos) {
			event.reply("That is not allowed here. Please, mind your language!",
						true);
		}
	});

	bot.start(dpp::st_wait);
	*/
	return 0;
}

int registerSlashCommands(dpp::cluster &bot) {
	dpp::slashcommand join("join", "Join a voice channel.", bot.me.id);
	dpp::slashcommand leave("leave", "Leave the voice channel.", bot.me.id);
	dpp::slashcommand play("play", "Play a song.", bot.me.id);

	return 0;
}