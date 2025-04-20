#include "play.h"
#include "../global.h"
#include "../lib/embeds.h"

void handlePlayerEventsOnDiscord(Player &player, const std::string &guildId) {
	player.onTrackStart([guildId](std::string data) {
		dpp::message msg;
		msg.content = "Now playing: " + data;
		msg.channel_id = GQ.queue[guildId].textChannelId;
		msg.guild_id = dpp::snowflake(guildId);
		BH.bot->message_create(msg, [guildId](const dpp::confirmation_callback_t &event) {});
	});
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

void Play(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	event.thinking(false, [](const dpp::confirmation_callback_t &) {});

	std::string guildId = event.command.guild_id.str();

	if (getUserVoiceChannel(event).empty()) {
		event.edit_original_response(dpp::message("You need to be in a voice channel to use this command."));
		return;
	}

	GQ.add(guildId);

	std::string query = std::get<std::string>(event.get_parameter("query"));

	LavaLink *node = LC.getIdealNode();
	nlohmann::json res = node->loadTracks(query);

	std::string loadType = "";
	if (res.contains("loadType") && res["loadType"].is_string()) {
		loadType = res["loadType"].get<std::string>();
	}

	nlohmann::json trackList = nlohmann::json::array();

	if (loadType == "empty") {
		try {
			res = node->loadTracks("ytsearch:" + query);
			if (res.contains("loadType") && res["loadType"].is_string()) {
				loadType = res["loadType"].get<std::string>();
				if (loadType == "search" && res.contains("data") && res["data"].is_array()) {
					trackList.push_back(res["data"][0]);
				}
			}
		} catch (const std::exception &e) {
			error(std::string("Exception during node->loadTracks (ytsearch retry): ") + e.what());
			loadType = "error";
		}
	} else if (loadType == "track" || loadType == "short") {
		if (res.contains("data") && res["data"].is_object() && res["data"].contains("info")) {
			trackList.push_back(res["data"][0]);
		} else if (res.contains("data") && res["data"].is_array() && !res["data"].empty() && res["data"][0].is_object() && res["data"][0].contains("info")) {
			trackList.push_back(res["data"][0]);
		} else {
			warn("loadType is '" + loadType + "' but track data structure is unexpected.");
		}
	} else if (loadType == "playlist" || loadType == "album" || loadType == "artist" || loadType == "station" || loadType == "podcast" || loadType == "show") {
		if (res.contains("data") && res["data"].is_object() && res["data"].contains("tracks") && res["data"]["tracks"].is_array()) {
			trackList = res["data"]["tracks"];
		} else {
			warn("loadType is '" + loadType + "' but tracks array is missing or invalid.");
		}
	} else if (loadType == "search") {
		if (res.contains("data") && res["data"].is_array()) {
			trackList = res["data"][0];
		}
	} else if (loadType == "error") {
		error("Lavalink returned loadType 'error' for query: " + query);
		event.reply("Sorry, couldn't find anything for that query.");
		return;
	}

	for (const nlohmann::json &track: trackList) {
		GQ.queue[guildId].add(track);
	}

	if (GQ.queue[guildId].voiceChannelId != getUserVoiceChannel(event)) {
		GQ.queue[guildId].voiceChannelId = getUserVoiceChannel(event);
		GQ.queue[guildId].textChannelId = event.command.channel_id.str();
		node->join(guildId, getUserVoiceChannel(event), false, false);
	}
	node->getPlayer(guildId).removeAllTrackStartListeners();
	handlePlayerEventsOnDiscord(node->getPlayer(guildId), guildId);
	node->getPlayer(guildId).volume((20));

	event.edit_original_response(dpp::message("Added " + std::to_string(trackList.size()) + " tracks to the queue."));
	if (GQ.queue[guildId].queue.size() > 0) {
		nlohmann::json t = GQ.queue[guildId].getNextTrack();
		node->getPlayer(guildId).play(t["encoded"]);
	} else {
		event.edit_original_response(dpp::message("No tracks found."));
	}

	return;
}