#include "decoration.h"

void beautifulLine(int width = 50, char symbol = '*', int speed_ms = 30, bool colorful = true) {
	const std::string colors[] = {
		"\033[31m",
		"\033[33m",
		"\033[32m",
		"\033[36m",
		"\033[34m",
		"\033[35m"};
	const std::string reset = "\033[0m";

	std::cout << std::endl;

	for (int i = 0; i < width; ++i) {
		if (colorful) {
			int colorIndex = i % 6;
			std::cout << colors[colorIndex] << symbol << reset << std::flush;
		} else {
			std::cout << symbol << std::flush;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(speed_ms));
	}

	std::cout << std::endl;
}