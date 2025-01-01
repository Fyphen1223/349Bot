#include "slashcommandsCreate.h"

std::string getRestPing(dpp::cluster &bot) {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(bot.rest_ping));
	return std::to_string(ms.count()) + " ms";
}


void executePing(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	event.reply("Pong! REST Ping: " + getRestPing(bot));
}

void executePlay(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executePause(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeResume(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeSearch(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeStop(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeSkip(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeBack(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeVolume(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeQueue(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeNowPlaying(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeLoop(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeShuffle(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeLyrics(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeSeek(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}


void onSlashCommands(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	const std::string commandName = event.command.get_command_name();

	if (commandName == "ping") {
		executePing(bot, event);
	}
}