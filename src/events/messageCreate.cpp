#include "messageCreate.h"

void onMessageCreate(dpp::cluster &bot, const dpp::message_create_t &event) {
	/*
	const std::string &content = event.msg.content;
	if (event.msg.author.is_bot())
		return;
	if (content.find("bad word") != std::string::npos) {
		event.reply("That is not allowed here. Please, mind your language!", false);
	}

	if (content.find("search") != std::string::npos) {
		std::string query = content.substr(content.find("search") + 7);
		event.reply("Searching for: " + query, false);
		LavaLink *node = LC.getIdealNode();
		nlohmann::json res = node->loadTracks("ytsearch:" + query);
		std::cout << res.dump(4) << std::endl;
		event.reply("Found: " + res["data"][0]["info"]["title"].get<std::string>(), false);
		dpp::message msg;
		msg.add_file("data.json", res.dump(4));
		msg.add_embed(dpp::embed().set_title("Search results").set_description("Here are the search results."));
		event.reply(msg);
	}

	if (content.find("play") != std::string::npos) {
		std::string query = content.substr(content.find("play") + 5);
		event.reply("Searching for: " + query, false);
		LavaLink *node = LC.getIdealNode();
		nlohmann::json res = node->loadTracks("ytsearch:" + query);
		event.reply("Playing: " + res["data"][0]["info"]["title"].get<std::string>(), false);
		node->join(std::to_string(event.msg.guild_id), std::to_string(event.msg.channel_id), true, true);
		Player &p = node->getPlayer(std::to_string(event.msg.guild_id));
		p.play(res["data"][0]["encoded"].get<std::string>());
		p.volume(20);
	}

	if (content.find("pause") != std::string::npos) {
		LavaLink *node = LC.getIdealNode();
		try {
			Player &p = node->getPlayer(std::to_string(event.msg.guild_id));
			p.pause();
			event.reply("Paused the player.", false);
		} catch (std::runtime_error &e) {
			event.reply("Player not found.", false);
		}
	}

	if (content.find("resume") != std::string::npos) {
		LavaLink *node = LC.getIdealNode();
		try {
			Player &p = node->getPlayer(std::to_string(event.msg.guild_id));
			p.resume();
			event.reply("Resumed the player.", false);
		} catch (std::runtime_error &e) {
			event.reply("Player not found.", false);
		}
	}
		*/
}