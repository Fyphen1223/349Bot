#include <iostream>

#include "print.h"

void print(const std::string &message) {
	std::cout << message << std::endl;
	return;
}

void print(const json &object) {
	std::cout << object.dump(4) << std::endl;
	return;
}

void print(const char *message) {
	std::cout << message << std::endl;
	return;
}

void print(const double &number) {
	print(std::to_string(number));
	return;
}