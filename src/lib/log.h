#ifndef LOG_H
#define LOG_H

#include "../global.h"
#include <dpp/dpp.h>
#include <iostream>

void error(const std::string &message);
void warn(const std::string &message);
void info(const std::string &message);
void logDebug(const std::string &message);
void logTrace(const std::string &message);
void logCritical(const std::string &message);
void logUnknown(const std::string &message);
void DiscordLogger(const dpp::log_t &log);
void setLogLevel(int level);

#endif// LOGGER_H