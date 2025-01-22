#ifndef SLASHCOMMANDSCREATE_H
#define SLASHCOMMANDSCREATE_H

#include <dpp/dpp.h>

#include <functional>
#include <iostream>
#include <map>

void onSlashCommands(dpp::cluster &bot, const dpp::slashcommand_t &event);
std::string getRestPing(dpp::cluster &bot);

#endif// SLASHCOMMANDCREATE_H