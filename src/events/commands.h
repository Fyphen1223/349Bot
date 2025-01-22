#ifndef COMMANDS_H
#define COMMANDS_H

#include <dpp/dpp.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "../commands/ping.h"

#include <functional>
#include <map>

extern std::map<std::string, std::function<void(dpp::cluster &bot, const dpp::slashcommand_t &event)>> Commands;
void initializeCommands();

#endif// COMMANDS_H