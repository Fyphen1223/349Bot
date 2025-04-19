#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include "../global.h"
#include "../lavacop/lavacop.h"
#include "../lib/log.h"
#include <dpp/dpp.h>
#include <nlohmann/json.hpp>

void onAutoComplete(dpp::cluster &bot, const dpp::autocomplete_t &event);

#endif// AUTOCOMPLETE_H