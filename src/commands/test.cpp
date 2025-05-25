#include "test.h"
#include "../global.h"
#include <dpp/dpp.h>
#include <iostream>
#include <string>

void Test(dpp::cluster &bot, const dpp::slashcommand_t &event) {
	event.reply(dpp::message()
					.set_flags(dpp::m_using_components_v2)
					.add_component_v2(
						/* Reply with a container... */
						dpp::component()
							.set_type(dpp::cot_container)
							.set_accent(dpp::utility::rgb(255, 255, 255))
							.set_spoiler(true)
							.add_component_v2(
								dpp::component()
									.set_type(dpp::cot_section)
									.add_component_v2(
										dpp::component()
											.set_type(dpp::cot_text_display)
											.set_content("Click if you love cats"))
									.set_accessory(
										dpp::component()
											.set_type(dpp::cot_button)
											.set_label("Pause")
											.set_style(dpp::cos_primary)
											.set_id("pause"))
									.set_accessory(
										dpp::component()
											.set_type(dpp::cot_button)
											.set_label("Back")
											.set_style(dpp::cos_secondary)
											.set_id("back"))
									.set_accessory(
										dpp::component()
											.set_type(dpp::cot_button)
											.set_label("skip")
											.set_style(dpp::cos_primary)
											.set_id("skip"))))
					.add_component_v2(
						dpp::component()
							.set_type(dpp::cot_separator)
							.set_spacing(dpp::sep_large)
							.set_divider(true))
					.add_component_v2(
						dpp::component()
							.set_type(dpp::cot_media_gallery)
							.add_media_gallery_item(
								dpp::component()
									.set_type(dpp::cot_thumbnail)
									.set_description("A cat")
									.set_thumbnail("https://www.catster.com/wp-content/uploads/2023/11/Beluga-Cat-e1714190563227.webp"))));
	return;
}