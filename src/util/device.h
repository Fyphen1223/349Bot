#ifndef DEVICE_H
#define DEVICE_H

#include "../global.h"
#include <dpp/dpp.h>
#include <string>

std::string getOS();
std::string getCPU();
std::string getRAM();
std::string getDisk();
std::string getKernel();
std::string getUptime();
std::string getHostname();
std::string getIP();

#endif// LOGGER_H