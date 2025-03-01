#include "device.h"
#include <string>
#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	#include <intrin.h>
	#define OS_NAME "Windows"

std::string getOS() {
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return std::string(OS_NAME) + " " + std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);
}

std::string getCPU() {
	int cpuInfo[4] = {-1};
	char cpuBrandString[0x40];
	__cpuid(cpuInfo, 0x80000000);
	unsigned int nExIds = cpuInfo[0];
	memset(cpuBrandString, 0, sizeof(cpuBrandString));
	for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
		__cpuid(cpuInfo, i);
		if (i == 0x80000002)
			memcpy(cpuBrandString, cpuInfo, sizeof(cpuInfo));
		else if (i == 0x80000003)
			memcpy(cpuBrandString + 16, cpuInfo, sizeof(cpuInfo));
		else if (i == 0x80000004)
			memcpy(cpuBrandString + 32, cpuInfo, sizeof(cpuInfo));
	}
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int numCores = sysInfo.dwNumberOfProcessors;
	return std::string(cpuBrandString) + " (" + std::to_string(numCores) + " cores)";
}

std::string getRAM() {
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (GlobalMemoryStatusEx(&statex)) {
		std::ostringstream oss;
		oss << (statex.ullTotalPhys / (1024 * 1024 * 1024)) << "GB";
		return oss.str();
	} else {
		throw std::runtime_error("Cannot get RAM information");
	}
}

#elif defined(__APPLE__) || defined(__MACH__)
	#include <sys/utsname.h>
	#include <sys/sysctl.h>
	#define OS_NAME "MacOS"

std::string getOS() {
	struct utsname buffer;
	uname(&buffer);
	return std::string(OS_NAME) + " " + std::string(buffer.release);
}

std::string getCPU() {
	char buffer[256];
	size_t bufferlen = sizeof(buffer);
	sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0);
	int numCores;
	size_t len = sizeof(numCores);
	sysctlbyname("hw.ncpu", &numCores, &len, NULL, 0);
	return std::string(buffer) + " (" + std::to_string(numCores) + " cores)";
}

std::string getRAM() {
	int64_t mem;
	size_t len = sizeof(mem);
	if (sysctlbyname("hw.memsize", &mem, &len, NULL, 0) == 0) {
		std::ostringstream oss;
		oss << (mem / (1024 * 1024)) << " MB";
		return oss.str();
	} else {
		throw std::runtime_error("RAM情報を取得できません");
	}
}

#elif defined(__linux__)
	#include <sys/utsname.h>
	#include <fstream>
	#include <sys/sysinfo.h>
	#include <sys/statvfs.h>
	#include <ifaddrs.h>
	#define OS_NAME "Linux"
	#include <linux/if_ether.h>
	#include <linux/sockios.h>
	#include <sys/ioctl.h>
	#include <net/if.h>

std::string getOS() {
	struct utsname buffer;
	uname(&buffer);
	return std::string(OS_NAME) + " " + std::string(buffer.release);
}

std::string getCPU() {
	std::ifstream cpuinfo("/proc/cpuinfo");
	std::string line;
	std::string modelName;
	int numCores = 0;
	while (std::getline(cpuinfo, line)) {
		if (line.find("model name") != std::string::npos) {
			modelName = line.substr(line.find(":") + 2);
		}
		if (line.find("cpu cores") != std::string::npos) {
			numCores = std::stoi(line.substr(line.find(":") + 2));
		}
	}
	return modelName + " (" + std::to_string(numCores) + " cores)";
}

std::string getRAM() {
	struct sysinfo info;
	if (sysinfo(&info) == 0) {
		std::ostringstream oss;
		oss << (info.totalram / (1024 * 1024 * 1024)) << "GB";
		return oss.str();
	} else {
		throw std::runtime_error("Cannot get RAM information");
	}
}

std::string getDisk() {
	struct statvfs buffer;
	if (statvfs("/", &buffer) == 0) {
		std::ostringstream oss;
		oss << (buffer.f_bsize * buffer.f_blocks) / (1024 * 1024 * 1024) << "GB";
		return oss.str();
	} else {
		throw std::runtime_error("Cannot get disk information");
	}
}

std::string getKernel() {
	struct utsname buffer;
	uname(&buffer);
	return std::string(buffer.release);
}

std::string getUptime() {
	struct sysinfo info;
	if (sysinfo(&info) == 0) {
		std::ostringstream oss;
		oss << (info.uptime) << " seconds";
		return oss.str();
	} else {
		throw std::runtime_error("Cannot get uptime information");
	}
}

std::string getHostname() {
	char buffer[256];
	if (gethostname(buffer, sizeof(buffer)) == 0) {
		return std::string(buffer);
	} else {
		throw std::runtime_error("Cannot get hostname information");
	}
}

std::string getIP() {
	char buffer[256];
	struct addrinfo hints, *info, *p;
	int gai_result;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((gai_result = getaddrinfo(getHostname().c_str(), "http", &hints, &info)) != 0) {
		throw std::runtime_error("Cannot get IP address information");
	}

	for (p = info; p != NULL; p = p->ai_next) {
		void *addr;
		std::string ipver;

		if (p->ai_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		inet_ntop(p->ai_family, addr, buffer, sizeof buffer);
		freeaddrinfo(info);
		return std::string(buffer);
	}

	freeaddrinfo(info);
	throw std::runtime_error("Cannot get IP address information");
}


#elif defined(__unix__)
	#include <sys/utsname.h>
	#include <fstream>
	#define OS_NAME "Unix"

std::string getOS() {
	struct utsname buffer;
	uname(&buffer);
	return std::string(OS_NAME) + " " + std::string(buffer.release);
}

std::string getCPU() {
	std::ifstream cpuinfo("/proc/cpuinfo");
	std::string line;
	std::string modelName;
	int numCores = 0;
	while (std::getline(cpuinfo, line)) {
		if (line.find("model name") != std::string::npos) {
			modelName = line.substr(line.find(":") + 2);
		}
		if (line.find("cpu cores") != std::string::npos) {
			numCores = std::stoi(line.substr(line.find(":") + 2));
		}
	}
	return modelName + " (" + std::to_string(numCores) + " cores)";
}

std::string getRAM() {
	struct sysinfo info;
	if (sysinfo(&info) == 0) {
		std::ostringstream oss;
		oss << (info.totalram / (1024 * 1024 * 1024)) << "GB";
		return oss.str();
	} else {
		throw std::runtime_error("Cannot get RAM information");
	}
}

#else
	#define OS_NAME "Unknown"

std::string getOS() {
	return "Unknown";
}

std::string getCPU() {
	return "Unknown";
}

std::string getRAM() {
	throw std::runtime_error("Unknown");
}
#endif