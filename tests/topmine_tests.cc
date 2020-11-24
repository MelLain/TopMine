// Author: Murat Apishev (@mel-lain)

#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"

#include "gtest/gtest.h"

#include "include/batch.h"
#include "include/topmine_impl.h"
#include "include/utils.h"

const std::string kInputPath = "../tests/test_data/test_data.txt";

std::pair<std::string, std::string> prepare_paths() {
  boost::filesystem::path test_directory_path("topmine_test_dir");
  boost::filesystem::create_directory(test_directory_path);

  auto output_path = test_directory_path;
  output_path.append("test_results_output.txt");

  auto collocations_output_path = test_directory_path;
  collocations_output_path.append("test_collocations_output.txt");

  return std::make_pair(output_path.string(), collocations_output_path.string());
}

void check_results(const std::pair<std::string, std::string>& output_paths, bool return_indices) {
  Batch batch(" \t");
  std::ifstream result_stream(output_paths.first);

  while (!result_stream.eof()) {
    std::string str;
    std::getline(result_stream, str);

    if (!str.empty()) {
      batch.add_document(str);
    }
  }
  ASSERT_EQ(batch.size(), 7);

  for (const auto& document : batch.get_documents()) {
    std::string result_str = Utils::join_strings(document.tokens, ' ');

    switch (document.id) {
      case 1L:
        ASSERT_EQ(result_str, return_indices
          ? "0|3 3|1 4|1 5|1" : "метод|опорных|векторов я любил использовать");
        break;

      case 2L:
        ASSERT_EQ(result_str, return_indices
          ? "0|1 1|1 2|1 3|1 4|4" : "ты работаешь лучше чем метод|опорных|векторов|ща");
        break;

      case 3L:
        ASSERT_EQ(result_str, return_indices
          ? "0|1 1|1 2|3 5|1 6|1" : "мой лучше метод|опорных|векторов лучше твоего");
        break;

      case 4L:
        ASSERT_EQ(result_str, return_indices
          ? "0|3 3|1 4|1 5|1 6|1 7|1" : "метод|опорных|векторов опорных самый лучший на земле");
        break;

      case 5L:
        ASSERT_EQ(result_str, return_indices
          ? "0|2 2|3 5|1" : "а|ты метод|опорных|векторов выучил");
        break;

      case 6L:
        ASSERT_EQ(result_str, return_indices
          ? "0|2 2|1 3|1 4|1 5|1 6|1 7|4" : "а|ты любишь заменять ядра когда юзаешь метод|опорных|векторов|ща");
        break;

      case 7L:
        ASSERT_EQ(result_str, return_indices
          ? "0|2 2|1 3|1 4|1 5|1 6|1 7|4" : "а|ты используешь rbf ядра когда используешь метод|опорных|векторов|ща");
        break;

      default:
        ASSERT_TRUE(false);
        break;
    }
  }

  int num_collocations = 0;
  std::ifstream collocations_stream(output_paths.second);

  std::unordered_map<std::string, int> collocation_to_df = {
    {"а|ты", 3},
    {"метод|опорных|векторов|ща", 3},
    {"опорных", 1},
    {"лучше", 2},
    {"метод|опорных|векторов", 4}
  };

  while (!collocations_stream.eof()) {
    std::string str;
    std::getline(collocations_stream, str);

    if (!str.empty()) {
      std::vector<std::string> parts;
      boost::split(parts, str, boost::is_any_of(" "));

      ASSERT_EQ(parts.size(), 2);

      auto iter = collocation_to_df.find(parts[0]);
      ASSERT_TRUE(iter != collocation_to_df.end());
      ASSERT_EQ(std::stoi(parts[1]), iter->second);

      ++num_collocations;
    }
  }

  ASSERT_EQ(num_collocations, collocation_to_df.size());
}

TEST(TopmineTests, BaseTest) {
  auto output_paths = prepare_paths();

  bool return_indices = false;
  Parameters parameters = {
    kInputPath,           // input_path
    output_paths.first,   // output_path
    output_paths.second,  // collocations_output_path
    4,                    // collocation_max_size
    1,                    // num_threads
    2,                    // batch_size
    3,                    // threshold
    0.01,                 // alpha
    return_indices,       // return_indices
    false,                // use_cache
    " \t",                // delimiters
    '|'                   // esc_character
  };

  TopmineImpl::run_topmine(parameters);

  check_results(output_paths, return_indices);
}

TEST(TopmineTests, MultithreadedTest) {
  auto output_paths = prepare_paths();

  bool return_indices = false;
  Parameters parameters = {
    kInputPath,           // input_path
    output_paths.first,   // output_path
    output_paths.second,  // collocations_output_path
    4,                    // collocation_max_size
    3,                    // num_threads
    2,                    // batch_size
    3,                    // threshold
    0.01,                 // alpha
    return_indices,       // return_indices
    false,                // use_cache
    " \t",                // delimiters
    '|'                   // esc_character
  };

  TopmineImpl::run_topmine(parameters);

  check_results(output_paths, return_indices);
}

TEST(TopmineTests, CacheTest) {
  auto output_paths = prepare_paths();

  bool return_indices = false;
  Parameters parameters = {
    kInputPath,           // input_path
    output_paths.first,   // output_path
    output_paths.second,  // collocations_output_path
    4,                    // collocation_max_size
    1,                    // num_threads
    2,                    // batch_size
    3,                    // threshold
    0.01,                 // alpha
    return_indices,       // return_indices
    true,                 // use_cache
    " \t",                // delimiters
    '|'                   // esc_character
  };

  TopmineImpl::run_topmine(parameters);

  check_results(output_paths, return_indices);
}

TEST(TopmineTests, TooManyThreadsTest) {
  auto output_paths = prepare_paths();

  bool return_indices = false;
  Parameters parameters = {
    kInputPath,           // input_path
    output_paths.first,   // output_path
    output_paths.second,  // collocations_output_path
    4,                    // collocation_max_size
    10,                   // num_threads
    10,                   // batch_size
    3,                    // threshold
    0.01,                 // alpha
    return_indices,       // return_indices
    true,                 // use_cache
    " \t",                // delimiters
    '|'                   // esc_character
  };

  TopmineImpl::run_topmine(parameters);

  check_results(output_paths, return_indices);
}

TEST(TopmineTests, ReturnIndicesTest) {
  auto output_paths = prepare_paths();

  bool return_indices = true;
  Parameters parameters = {
    kInputPath,           // input_path
    output_paths.first,   // output_path
    output_paths.second,  // collocations_output_path
    4,                    // collocation_max_size
    2,                    // num_threads
    2,                    // batch_size
    3,                    // threshold
    0.01,                 // alpha
    return_indices,       // return_indices
    true,                 // use_cache
    " \t",                // delimiters
    '|'                   // esc_character
  };

  TopmineImpl::run_topmine(parameters);

  check_results(output_paths, return_indices);
}
