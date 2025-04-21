#include "autocomplete.h"

void onAutoComplete(dpp::cluster &bot, const dpp::autocomplete_t &event) {
	if (event.name == "play") {
		for (auto &opt: event.options) {
			if (opt.focused) {
				std::string query = std::get<std::string>(opt.value);

				LavaLink *node = LC.getIdealNode();
				nlohmann::json res = node->loadTracks(query);

				std::string loadType = "";
				if (res.contains("loadType") && res["loadType"].is_string()) {
					loadType = res["loadType"].get<std::string>();
				}

				nlohmann::json trackList = nlohmann::json::array();
				int maxResults = 5;

				if (loadType == "empty") {
					try {
						res = node->loadTracks("ytsearch:" + query);
						if (res.contains("loadType") && res["loadType"].is_string()) {
							loadType = res["loadType"].get<std::string>();
							if (loadType == "search" && res.contains("data") && res["data"].is_array()) {
								trackList = res["data"];
								maxResults = 25;
							}
						}
					} catch (const std::exception &e) {
						error(std::string("Exception during node->loadTracks (ytsearch retry): ") + e.what());
						loadType = "error";
					}
				} else if (loadType == "track" || loadType == "short") {
					if (res.contains("data") && res["data"].is_object() && res["data"].contains("info")) {
						trackList.push_back(res["data"]);
						maxResults = 1;
					} else if (res.contains("data") && res["data"].is_array() && !res["data"].empty() && res["data"][0].is_object() && res["data"][0].contains("info")) {
						trackList.push_back(res["data"][0]);
						maxResults = 1;
					} else {
						warn("loadType is '" + loadType + "' but track data structure is unexpected.");
					}
				} else if (loadType == "playlist" || loadType == "album" || loadType == "artist" || loadType == "station" || loadType == "podcast" || loadType == "show") {
					if (res.contains("data") && res["data"].is_object() && res["data"].contains("tracks") && res["data"]["tracks"].is_array()) {
						trackList = res["data"]["tracks"];
						maxResults = 25;
					} else {
						warn("loadType is '" + loadType + "' but tracks array is missing or invalid.");
					}
				} else if (loadType == "search") {
					if (res.contains("data") && res["data"].is_array()) {
						trackList = res["data"];
						maxResults = 20;
					}
				} else if (loadType == "error") {
					error("Lavalink returned loadType 'error' for query: " + query);
				}

				dpp::interaction_response response(dpp::ir_autocomplete_reply);
				int count = 0;

				for (const auto &track: trackList) {
					if (count >= maxResults) {
						break;
					}

					try {
						if (track.contains("info") && track["info"].contains("title") && track.contains("encoded")) {
							std::string title = track["info"]["title"].get<std::string>();
							if (title.length() > 100) {
								title = title.substr(0, 97) + "...";
							}

							response.add_autocomplete_choice(dpp::command_option_choice(title, title));
							count++;
						}
					} catch (const nlohmann::json::exception &e) {
						error("Error parsing track info for autocomplete");
					} catch (const std::exception &e) {
						error("Error processing track for autocomplete");
					}
				}
				bot.interaction_response_create(
					event.command.id,
					event.command.token,
					response);
				return;
			}
		}
	}
}