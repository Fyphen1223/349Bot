#include "commands.h"
#include "../commands/ping.h"
#include "../commands/play.h"
#include "../commands/test.h"

std::map<std::string, std::function<void(dpp::cluster &bot, const dpp::slashcommand_t &event)>> Commands;

void initializeCommands() {
	Commands["ping"] = Ping;
	Commands["play"] = Play;
	Commands["test"] = Test;
}
