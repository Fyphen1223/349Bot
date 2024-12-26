#include "messageCreate.h"

void onMessageCreate(dpp::cluster &bot, const dpp::message_create_t &event /*, lavacop &LC*/) {
	const std::string &content = event.msg.content;
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
}