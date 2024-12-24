#include "slashcommandsCreate.h"

std::string getRestPing(dpp::cluster &bot) {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(bot.rest_ping));
	return std::to_string(ms.count()) + " ms";
}


void executePing(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	event.reply("Pong! REST Ping: " + getRestPing(bot));
}

void onSlashCommands(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	const std::string commandName = event.command.get_command_name();

	if (commandName == "ping") {
		executePing(bot, event);
	}
}