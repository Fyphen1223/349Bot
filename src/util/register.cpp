#include "register.h"

int registerSlashCommands(dpp::cluster &bot) {
	dpp::slashcommand ping("ping", "Ping command", bot.me.id);
	dpp::slashcommand join("join", "Join a voice channel", bot.me.id);
	dpp::slashcommand leave("leave", "Leave the voice channel", bot.me.id);
	dpp::slashcommand play("play", "Playback things", bot.me.id);
	play.add_option(dpp::command_option(dpp::co_string, "query", "The query used for playback", false).set_auto_complete((true)));

	dpp::slashcommand pause("pause", "Pause the current track", bot.me.id);
	dpp::slashcommand resume("resume", "Resume the current track", bot.me.id);
	dpp::slashcommand skip("skip", "Skip the current track", bot.me.id);
	dpp::slashcommand stop("stop", "Stop playback and clear the queue", bot.me.id);
	dpp::slashcommand back("back", "Go back to the previous track", bot.me.id);
	dpp::slashcommand queue("queue", "Show the current playback queue", bot.me.id);

	dpp::slashcommand test("test", "Test command for debugging", bot.me.id);

	bot.global_bulk_command_create({ping, join, leave, play, pause, resume, skip, stop, back, queue, test});
	info("Slash commands registered.");
	return 0;
}