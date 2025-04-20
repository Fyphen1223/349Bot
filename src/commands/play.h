#ifndef PLAY_H
#define PLAY_H

#include "../global.h"
#include "../lib/log.h"
#include <dpp/dpp.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include <dpp/dpp.h>

void Play(dpp::cluster &bot, const dpp::slashcommand_t &event);

#endif// PLAY_H