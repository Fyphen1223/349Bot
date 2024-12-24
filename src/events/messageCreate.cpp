#include "messageCreate.h"

void onMessageCreate(dpp::cluster &bot, const dpp::message_create_t &event) {
	if (event.msg.content.find("bad word") != std::string::npos) {
		event.reply("That is not allowed here. Please, mind your language!", false);
	}
}