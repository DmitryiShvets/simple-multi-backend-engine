#pragma once
#include <string>

class Logger {
public:
	static void error_log(const std::string& description);
	static void info_log(const std::string& description);
	static void warning_log(const std::string& description);
};
