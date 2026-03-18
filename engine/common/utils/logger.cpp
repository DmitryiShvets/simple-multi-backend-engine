#include "utils/logger.h"
#include <iostream>
namespace ssme {

void Logger::error_log(const std::string &description) {
  std::cerr << "[ERROR] " << description << std::endl;
}
void Logger::validation_log(const std::string &type,
                            const std::string &description) {
  std::cerr << "[VALIDATION LAYER] type:" << type << " msg: " << description
            << std::endl;
}
void Logger::info_log(const std::string &description) {
  std::cerr << "[INFO] " << description << std::endl;
}
void Logger::warning_log(const std::string &description) {
  std::cerr << "[Warning] " << description << std::endl;
}

// void Logger::error_log(std::string_view description) {
//   std::cerr << "[ERROR] " << description << std::endl;
// }
// void Logger::info_log(std::string_view description) {
//   std::cerr << "[INFO] " << description << std::endl;
// }
// void Logger::warning_log(std::string_view description) {
//   std::cerr << "[Warning] " << description << std::endl;
// }
} // namespace ssme
