// Author: Murat Apishev (@mel-lain)

#pragma once

#include <string>
#include <vector>

class Utils {
 public:
  static std::string join_strings(const std::vector<std::string>& strings, char separator);

  static std::string join_strings(const std::vector<std::string>& strings,
                                  int begin_index,
                                  int end_index,
                                  char separator);

  static long get_peak_memory_usage_kb();
};
