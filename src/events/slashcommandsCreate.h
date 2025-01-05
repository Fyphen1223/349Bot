#ifndef SLASHCOMMANDSCREATE_H
#define SLASHCOMMANDSCREATE_H

#include <dpp/dpp.h>

#include <iostream>

void onSlashCommands(dpp::cluster &bot, const dpp::slashcommand_t &event);
std::string getRestPing(dpp::cluster &bot);

#endif// SLASHCOMMANDCREATE_H