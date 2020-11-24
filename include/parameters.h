// Author: Murat Apishev (@mel-lain)

#pragma once

#include <string>

struct Parameters {
  std::string input_path;
  std::string output_path;
  std::string collocations_output_path;
  int collocation_max_size;
  int num_threads;
  int batch_size;
  int threshold;
  float alpha;
  bool return_indices;
  bool use_cache;
  std::string delimiters;
  char esc_character;
};
