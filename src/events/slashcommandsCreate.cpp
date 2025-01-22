#include "slashcommandsCreate.h"
#include "../global.h"
#include "../lavacop/lavacop.h"
#include "../lavacop/src/lavalink.h"
#include <dpp/dpp.h>
#include <functional>
#include <iostream>
#include <map>

#include "../commands/ping.h"
#include "commands.h"

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

void handlePlayerEventsOnDiscord(Player &player) {
	player.onTrackStart([](std::string data) {
		std::cout << "Track started: " << data << std::endl;
	});
}

void executePlay(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	const std::string query = std::get<std::string>(event.get_parameter("query"));
	const std::string guildId = std::to_string(event.command.get_guild().id);

	GQ.add(std::to_string(event.command.get_guild().id));
	if (getUserVoiceChannel(event).empty()) {
		event.reply("You need to be in a voice channel to use this command.");
		return;
	}


	if (query.empty()) {
		event.reply("Please provide a query.");
		return;
	}

	std::thread([event]() {
		event.thinking(false);
	}).detach();

	LavaLink *node = LC.getIdealNode();
	try {
		LC.getPlayer(std::to_string(event.command.get_guild().id));
		if (GQ.queue[guildId].voiceChannelId != getUserVoiceChannel(event)) {
			GQ.queue[guildId].voiceChannelId = getUserVoiceChannel(event);
			node->join(std::to_string(event.command.get_guild().id), getUserVoiceChannel(event), false, false);
		}
	} catch (std::runtime_error &e) {
		GQ.queue[guildId].voiceChannelId = getUserVoiceChannel(event);
		node->join(std::to_string(event.command.get_guild().id), getUserVoiceChannel(event), false, false);
		handlePlayerEventsOnDiscord(node->getPlayer(std::to_string(event.command.get_guild().id)));
	}

	GQ.queue[guildId].voiceChannelId = getUserVoiceChannel(event);
	//Joijns the voice channel

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

	if (node->getPlayer(std::to_string(event.command.get_guild().id)).isNoEventListenerSet()) {
		handlePlayerEventsOnDiscord(node->getPlayer(std::to_string(event.command.get_guild().id)));
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

void onSlashCommands(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	const std::string commandName = event.command.get_command_name();

	if (Commands.find(commandName) != Commands.end()) {
		Commands[commandName](bot, event);
	} else {
		event.reply("Command not found.");
	}
}