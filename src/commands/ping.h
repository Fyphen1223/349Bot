#ifndef PING_H
#define PING_H

#include "../global.h"
#include <dpp/dpp.h>
#include <iostream>
#include <string>

std::string getRestPing(dpp::cluster &bot) {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(bot.rest_ping));
	return std::to_string(ms.count()) + " ms";
}

extern void Ping(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	event.reply("Pong! REST Ping: " + getRestPing(bot));
}


#endif// PING_H