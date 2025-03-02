#include "log.h"
#include "../global.h"
#include "print.h"
#include <filesystem>
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

std::string logDirectory = "logs";

void setLogDirectory(const std::string &directory) {
	logDirectory = directory;
}

std::string getCurrentTime() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << '.' << std::setfill('0') << std::setw(3) << ms.count();

	// Get timezone offset
	std::time_t t = std::time(nullptr);
	std::tm local_tm = *std::localtime(&t);
	std::tm utc_tm = *std::gmtime(&t);
	int offset = local_tm.tm_hour - utc_tm.tm_hour;
	if (local_tm.tm_yday != utc_tm.tm_yday) {
		offset += (local_tm.tm_yday > utc_tm.tm_yday) ? 24 : -24;
	}

	ss << " UTC" << (offset >= 0 ? "+" : "") << offset;
	return ss.str();
}

std::string getFormattedTime() {
	return "[" + getCurrentTime() + "]";
}

int logLevel = 2;

void writeLog(const std::string &message) {
	if (!std::filesystem::exists(logDirectory)) {
		std::filesystem::create_directories(logDirectory);
	}
	std::string logPath = logDirectory + "/log.txt";
	std::ofstream logFile(logPath, std::ios::app);
	if (logFile.is_open()) {
		logFile << message << std::endl;
		logFile.close();
	} else {
		std::cerr << "Failed to open log file: " << logPath << std::endl;
	}
	return;
}

int maxLogFiles = 10;// Default to keeping 10 log files

void setMaxLogFiles(int max) {
	maxLogFiles = max;
}

void initiateLog() {
	// Ensure log directory exists
	if (!std::filesystem::exists(logDirectory)) {
		std::filesystem::create_directories(logDirectory);
	}

	std::string logPath = logDirectory + "/log.txt";
	bool oldLogExists = std::filesystem::exists(logPath);

	// If old log file exists, rename it using its first line timestamp
	if (oldLogExists) {
		std::ifstream oldLog(logPath);
		if (oldLog.is_open()) {
			std::string timestamp;
			std::getline(oldLog, timestamp);// First line contains the timestamp
			oldLog.close();

			// Clean up timestamp for filename
			std::string cleanTimestamp = timestamp;
			for (char &c: cleanTimestamp) {
				if (c == ':' || c == ' ' || c == '.' || c == '+' || c == '-') {
					c = '_';
				}
			}

			std::string newLogName = logDirectory + "/log_" + cleanTimestamp + ".txt";
			try {
				std::filesystem::rename(logPath, newLogName);
			} catch (const std::filesystem::filesystem_error &e) {
				std::cerr << "Failed to rename log file: " << e.what() << std::endl;
			}
		}
	}

	// Check the number of log files and delete oldest if necessary
	if (maxLogFiles > 0) {
		std::vector<std::filesystem::path> logFiles;

		// Get all log files except the current log.txt
		for (const auto &entry: std::filesystem::directory_iterator(logDirectory)) {
			if (entry.path().filename() != "log.txt" && entry.path().extension() == ".txt") {
				logFiles.push_back(entry.path());
			}
		}

		// If more than maxLogFiles log files exist, delete oldest
		if (logFiles.size() >= maxLogFiles) {
			// Sort files by last modification time
			std::sort(logFiles.begin(), logFiles.end(),
					  [](const std::filesystem::path &a, const std::filesystem::path &b) {
						  return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
					  });

			// Delete oldest files until we're under the limit
			size_t numToDelete = logFiles.size() - maxLogFiles + 1;// +1 for new log
			for (size_t i = 0; i < numToDelete; ++i) {
				try {
					std::filesystem::remove(logFiles[i]);
				} catch (const std::filesystem::filesystem_error &e) {
					std::cerr << "Failed to delete old log file: " << e.what() << std::endl;
				}
			}
		}
	}

	// Create new log file with current timestamp
	std::ofstream logFile(logPath, std::ios::out);
	if (logFile.is_open()) {
		logFile << getCurrentTime() << std::endl;
		logFile.close();
	} else {
		std::cerr << "Failed to open log file: " << logPath << std::endl;
	}
	return;
}

void error(const std::string &message) {
	if (4 >= logLevel) {
		print(BG_RED + "[ERROR]" + RESET + getFormattedTime() + ": " + message);
		writeLog("[ERROR]" + getFormattedTime() + ": " + message);
	}

	return;
}

void warn(const std::string &message) {
	if (3 >= logLevel) {
		print(BG_YELLOW + "[WARN ]" + RESET + getFormattedTime() + ": " + message);
		writeLog("[WARN ]" + getFormattedTime() + ": " + message);
	}
	return;
}

void info(const std::string &message) {
	if (2 >= logLevel) {
		print(BG_BLUE + "[INFO ]" + RESET + getFormattedTime() + ": " + message);
		writeLog("[INFO ]" + getFormattedTime() + ": " + message);
	}
	return;
}

void logDebug(const std::string &message) {
	if (1 >= logLevel) {
		print(BG_GREEN + "[DEBUG]" + RESET + getFormattedTime() + ": " + message);
		writeLog("[DEBUG]" + getFormattedTime() + ": " + message);
	}
	return;
}

void logTrace(const std::string &message) {
	if (0 >= logLevel) {
		print(BG_CYAN + "[TRACE]" + RESET + getFormattedTime() + ": " + message);
		writeLog("[TRACE]" + getFormattedTime() + ": " + message);
	}
	return;
}

void logCritical(const std::string &message) {
	if (5 >= logLevel) {
		print(BG_MAGENTA + "[CRITICAL]" + RESET + getFormattedTime() + ": " + message);
		writeLog("[CRITICAL]" + getFormattedTime() + ": " + message);
	}
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