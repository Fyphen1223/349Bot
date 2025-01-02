#include "register.h"

int registerSlashCommands(dpp::cluster &bot) {
	dpp::slashcommand ping("ping", "Ping command", bot.me.id);
	dpp::slashcommand join("join", "Join a voice channel", bot.me.id);
	dpp::slashcommand leave("leave", "Leave the voice channel", bot.me.id);
	dpp::slashcommand play("play", "Playback things", bot.me.id);
	play.add_option(dpp::command_option(dpp::co_string, "query", "The query used for playback", false).set_auto_complete((true)));

	bot.global_bulk_command_create({ping, join, leave, play});
	info("Slash commands registered.");
	return 0;
}