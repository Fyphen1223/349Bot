#include "commands.h"
#include "../commands/ping.h"

std::map<std::string, std::function<void(dpp::cluster &bot, const dpp::slashcommand_t &event)>> Commands;

void initializeCommands() {
	Commands["ping"] = Ping;
}
