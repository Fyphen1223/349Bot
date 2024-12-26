#include "log.h"
#include "print.h"
#include <iostream>

const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

const std::string BG_RED = "\033[41m";
const std::string BG_GREEN = "\033[42m";
const std::string BG_YELLOW = "\033[43m";
const std::string BG_BLUE = "\033[44m";
const std::string BG_MAGENTA = "\033[45m";
const std::string BG_CYAN = "\033[46m";
const std::string BG_WHITE = "\033[47m";

std::string getCurrentTime() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
	return ss.str();
}

std::string getFormattedTime() {
	return "[" + getCurrentTime() + "]";
}

int logLevel = 2;

void error(const std::string &message) {
	if (4 >= logLevel)
		print(BG_RED + "[ERRR]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void warn(const std::string &message) {
	if (3 >= logLevel)
		print(BG_YELLOW + "[WARN]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void info(const std::string &message) {
	if (2 >= logLevel)
		print(BG_BLUE + "[INFO]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logDebug(const std::string &message) {
	if (1 >= logLevel)
		print(BG_GREEN + "[DBUG]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logTrace(const std::string &message) {
	if (0 >= logLevel)
		print(BG_CYAN + "[TRCE]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logCritical(const std::string &message) {
	if (5 >= logLevel)
		print(BG_MAGENTA + "[CRIT]" + RESET + getFormattedTime() + ": " + message);
	return;
}

void logUnknown(const std::string &message) {
	print("[UNKNOWN]" + getFormattedTime() + ": " + message);
	return;
}

void DiscordLogger(const dpp::log_t &log) {
	std::string severity;
	switch (log.severity) {
		case dpp::ll_trace:
			severity = "TRACE";
			break;
		case dpp::ll_debug:
			severity = "DEBUG";
			break;
		case dpp::ll_info:
			severity = "INFO";
			break;
		case dpp::ll_warning:
			severity = "WARNING";
			break;
		case dpp::ll_error:
			severity = "ERROR";
			break;
		case dpp::ll_critical:
			severity = "CRITICAL";
			break;
		default:
			severity = "UNKNOWN";
			break;
	}

	if (severity == "TRACE") {
		logTrace(log.message);
	} else if (severity == "DEBUG") {
		logDebug(log.message);
	} else if (severity == "INFO") {
		info(log.message);
	} else if (severity == "WARNING") {
		warn(log.message);
	} else if (severity == "ERROR") {
		error(log.message);
	} else if (severity == "CRITICAL") {
		logCritical(log.message);
	} else {
		logUnknown(log.message);
	}
}

void setLogLevel(int level) {
	logLevel = level;
}