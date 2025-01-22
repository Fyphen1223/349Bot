#ifndef PING_H
#define PING_H

#include "../global.h"
#include <dpp/dpp.h>
#include <iostream>
#include <string>

#include <dpp/dpp.h>

std::string getRestPing(dpp::cluster &bot);
void Ping(dpp::cluster &bot, const dpp::slashcommand_t &event);

#endif// PING_H