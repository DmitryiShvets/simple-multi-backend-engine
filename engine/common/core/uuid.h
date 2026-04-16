#pragma once

#include <random>
#include <string>

namespace ssme {

inline std::string genUuidV4() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static const char *chars = "0123456789abcdef";

  std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

  for (char &c : uuid) {
    if (c == 'x') {
      c = chars[gen() % 16];
    } else if (c == 'y') {
      c = chars[(gen() % 4) + 8]; // Variant must be 8, 9, a or b
    }
  }
  return uuid;
}

} // namespace ssme
