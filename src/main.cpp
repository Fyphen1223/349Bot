#include <dpp/dpp.h>

#include <iostream>

int print(std::string message) {
	std::cout << message << std::endl;
	return 0;
}

int main() {
	/* Create the bot, but with our intents so we can use messages. */
	dpp::cluster bot(
		"MTI4MjUyMTY2NzQ1Njg2NDI5Ng.GC6b6V.jhNACAu1nZye5_qGxVieuC_"
		"Rjqn7VtNE7bUhGs",
		dpp::i_default_intents | dpp::i_message_content);

	bot.on_log(dpp::utility::cout_logger());

	/* The event is fired when the bot detects a message in any server and any
     * channel it has access to. */
	bot.on_message_create([&bot](const dpp::message_create_t &event) {
		/* See if the message contains the phrase we want to check for.
	 * If there's at least a single match, we reply and say it's not
	 * allowed.
	 */
		if (event.msg.content.find("bad word") != std::string::npos) {
			event.reply("That is not allowed here. Please, mind your language!",
						true);
		}
	});

	bot.start(dpp::st_wait);

	return 0;
}