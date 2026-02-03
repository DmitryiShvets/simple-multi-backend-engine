#include "logger.h"

 void Logger::error_log(const std::string& description)
{
	std::cerr << "Error: " << description << std::endl;
}
void Logger::info_log(const std::string& description)
{
	std::cerr << "Info: " << description << std::endl;
}
