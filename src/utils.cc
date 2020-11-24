// Author: Murat Apishev (@mel-lain)

#include <sys/time.h>
#include <sys/resource.h>

#include <sstream>

#include "include/utils.h"

std::string Utils::join_strings(const std::vector<std::string>& strings,
                                int begin_index,
                                int end_index,
                                char separator)
{
  std::stringstream ss;
  for (int i = begin_index; i < end_index; ++i) {
    ss << strings[i] << separator;
  }

  auto retval = ss.str();
  if (retval.size() > 0) {
    retval.pop_back();
  }

  return retval;
}

std::string Utils::join_strings(const std::vector<std::string>& strings, char separator) {
  return join_strings(strings, 0, strings.size(), separator);
}

long Utils::get_peak_memory_usage_kb() {
  rusage info;
  if (!getrusage(RUSAGE_SELF, &info)) {
    return info.ru_maxrss;
  }

  return 0;
}
