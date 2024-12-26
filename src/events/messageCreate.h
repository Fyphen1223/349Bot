#ifndef MESSAGECREATE_H
#define MESSAGECREATE_H

#include "../global.h"
#include "../lavacop/lavacop.h"
#include <dpp/dpp.h>
#include <nlohmann/json.hpp>

void onMessageCreate(dpp::cluster &bot, const dpp::message_create_t &event);

#endif// MESSAGECREATE_H