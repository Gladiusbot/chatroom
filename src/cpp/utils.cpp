#include "utils.h"

#include <memory>
#include <mutex>

bool start_with(const std::string& src, const char target) {
  if (src.size() == 0) {
    return false;
  }
  if (src[0] == target) {
    return true;
  }
  return false;
}