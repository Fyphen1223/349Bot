#include "textcommands.h"
#include "../lib/log.h"
#include <string>

void handleCLICommand(const std::string &command) {
	if (command == "help" || command == "h") {
		info("Available commands:");
		info("  help, h       - Display this help message");
		info("  exit, quit, q - Exit the application");
		info("  version, v    - Display version information");
		info("  status, s     - Display current bot status");
	} else if (command == "exit" || command == "quit" || command == "q") {
		info("Exiting application...");
		exit(0);
	} else if (command == "version" || command == "v") {
		info("Version 0.0.1");
	} else if (command == "status" || command == "s") {
		info("Bot is currently running");
	} else {
		warn("Unknown command: " + command);
		info("Type 'help' for a list of available commands");
	}
}