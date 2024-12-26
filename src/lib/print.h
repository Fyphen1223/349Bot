#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include <nlohmann/json.hpp>

void print(const std::string &message);
void print(const nlohmann::json &object);
void print(const char &message);
void print(const double &number);

#endif// PRINT_H