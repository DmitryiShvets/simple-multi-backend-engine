#pragma once
#include <iostream>
class Logger {
public:
	static void error_log(const std::string& description);
	static void info_log(const std::string& description);
};
