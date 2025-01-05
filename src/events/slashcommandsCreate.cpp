#include "slashcommandsCreate.h"
#include "../global.h"
#include <dpp/dpp.h>
#include <iostream>

std::string getRestPing(dpp::cluster &bot) {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(bot.rest_ping));
	return std::to_string(ms.count()) + " ms";
}

std::string getUserVoiceChannel(const dpp::slashcommand_t &event) {
	dpp::snowflake user_id = event.command.usr.id;
	dpp::guild *g = dpp::find_guild(event.command.guild_id);

	if (!g->id.empty()) {
		auto user_voice_state = g->voice_members.find(user_id);
		if (user_voice_state != g->voice_members.end()) {
			dpp::snowflake voice_channel_id = user_voice_state->second.channel_id;
			return std::to_string(voice_channel_id);
		} else {
			return "";
		}
	} else {
		return "";
	}
	return "";
}

void executePing(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	event.reply("Pong! REST Ping: " + getRestPing(bot));
}

void executePlay(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	const std::string query = std::get<std::string>(event.get_parameter("query"));
	const std::string guildId = std::to_string(event.command.get_guild().id);

	if (getUserVoiceChannel(event).empty()) {
		event.reply("You need to be in a voice channel to use this command.");
		return;
	}

	if (query.empty()) {
		event.reply("Please provide a query.");
		return;
	}

	event.thinking(false);

	auto node = LC.getIdealNode();
	auto track = node->loadTracks(query);

	std::string loadType = track["loadType"];

	if (loadType == "track" || loadType == "short") {
		track = track["data"];
	} else if (loadType == "empty") {
		track = node->loadTracks("ytsearch:" + query);
		loadType = track["loadType"];
		if (loadType == "empty" || loadType == "error") {
			event.edit_response("No tracks found.");
			return;
		} else {
			track = track["data"];
		}
	} else if (loadType == "search") {
		// search
	} else if (loadType == "error") {
		event.edit_response("An error occurred while trying to load the track.");
	} else if (loadType == "playlist" || loadType == "album" || loadType == "artist" || loadType == "station" || loadType == "podcast" || loadType == "show") {
		std::cout << track.dump() << std::endl;
		// playlist
	}

	GQ.add(std::to_string(event.command.get_guild().id));

	try {
		node->getPlayer(std::to_string(event.command.get_guild().id));
	} catch (std::runtime_error &e) {
		node->join(std::to_string(event.command.get_guild().id), getUserVoiceChannel(event), false, false);
	}

	GQ.queue[guildId].add(track[0].dump());

	event.edit_response(query + " has been loaded.");
	try {
		nlohmann::json t = GQ.queue[guildId].getNextTrack();
		LC.getPlayer(std::to_string(event.command.get_guild().id))->play(t["encoded"]);
	} catch (std::runtime_error &e) {
		event.edit_response("An error occurred while trying to play the track.");
	}
}

void executePause(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeResume(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeSearch(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeStop(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeSkip(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeBack(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeVolume(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeQueue(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeNowPlaying(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeLoop(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeShuffle(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeLyrics(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}

void executeSeek(dpp::cluster &bot, const dpp::slashcommand_t &event) {
}


void onSlashCommands(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	std::cout << "Slash command received: " << event.command.get_command_name() << std::endl;
	const std::string commandName = event.command.get_command_name();

	if (commandName == "ping") {
		executePing(bot, event);
	}

	if (commandName == "play") {
		executePlay(bot, event);
	}
}