// Author: Murat Apishev (@mel-lain)

#include <iostream>

#include <boost/filesystem.hpp>
#include "boost/program_options.hpp"

#include "include/common.h"
#include "include/parameters.h"
#include "include/topmine_impl.h"

namespace po = boost::program_options;

bool parse_parameters(int argc, char* argv[], Parameters* parameters) {
  po::options_description all_options("Options");
  all_options.add_options()
    ("help", "Show help\n")

    ("input-path",
      po::value(&parameters->input_path)->default_value(""),
      (std::string("Path to txt file with sentences.\n\n") +
       std::string("Each line is one sentence with tokens separated by any number of <delimiters> chars.\n") +
       std::string("First element in each line should be a long integer id of the sentence.\n") +
       std::string("The input file should not contain <esc_character>!\n") +
       std::string("Each line should contain at least an id and one token.\n")).c_str())

    ("output-path",
      po::value(&parameters->output_path)->default_value(""),
      (std::string("Path to file with resulting sentences.\n\n") +
       std::string("Will contain transformed input document in random order.\n") +
       std::string("Due to 'return-indices' parameter will be filled either with indices ") +
       std::string("of n-grams with format '<start_index_in_sentence>|<length_of_n_gram>' ") +
       std::string("or with n-grams, separated by <esc_character>.\n") +
       std::string("If 'output-path' is empty, than algorith won't transform documents.\n")).c_str())

    ("collocations-output-path",
      po::value(&parameters->collocations_output_path)->default_value("collocations.txt"),
      (std::string("Path to file for output collocations and their df\n\n") +
       std::string("Output line format: '<collocation> <df>', where tokens in collocation ") +
       std::string("are separated by <esc_character>.\n")).c_str())

    ("collocation-max-size",
      po::value(&parameters->collocation_max_size)->default_value(2),
      "Max size of collocations to search for.\n")

    ("num-threads",
      po::value(&parameters->num_threads)->default_value(1),
      "Number of parallel threads.\n")

    ("batch-size",
      po::value(&parameters->batch_size)->default_value(100),
      "Size of one portion for a thread.\n")

    ("threshold",
      po::value(&parameters->threshold)->default_value(0),
      "Min absolute occurrences to filter token/collocation.\n")

    ("alpha",
      po::value(&parameters->alpha)->default_value(2 * kEps),
      "Statistic significance threshold for final partition stage.\n")

    ("return-indices",
      po::value(&parameters->return_indices)->default_value(0),
      "Return indices of n-grams or source n-grams while transforming input documents.\n")

    ("use-cache",
      po::value(&parameters->use_cache)->default_value(0),
      "Use caching of source text during first pass through collection or not.\n")

    ("delimiters",
      po::value(&parameters->delimiters)->default_value(" "),
      "Characters to separate tokens from each other.\n")

    ("esc-character",
      po::value(&parameters->esc_character)->default_value('|'),
      "Escaped character for technical issues and output collocations representation.\n");

  po::variables_map variables_map;
  store(po::command_line_parser(argc, argv).options(all_options).run(), variables_map);
  notify(variables_map);

  bool show_help = (variables_map.count("help") > 0);
  if (show_help) {
    std::cerr << all_options;
    return true;
  }

  return false;
}

void check_parameters(const Parameters& parameters) {
  if (!boost::filesystem::exists(parameters.input_path)) {
    throw std::runtime_error("Error: input file does not exist: " + parameters.input_path);
  }

  if (parameters.num_threads <= 0) {
    throw std::runtime_error("Error: num_threads should be a positive integer");
  }

  if (parameters.collocation_max_size <= 0) {
    throw std::runtime_error("Error: collocation_max_size should be a positive integer");
  }

  if (parameters.threshold < 0) {
    throw std::runtime_error("Error: threshold should be a non-negative integer");
  }

  if (parameters.alpha < kEps) {
    throw std::runtime_error("Error: alpha should be a positive float");
  }
}

void print_parameters(const Parameters& parameters) {
  std::cout << std::endl << std::endl;
  std::cout << "================================================" << std::endl;
  std::cout << "Run Aithea TopMine with the following configuration: " << std::endl
            << "- path to file with sentences:              " << parameters.input_path << std::endl
            << "- max size of collocations to search:       " << parameters.collocation_max_size << std::endl
            << "- threshold for tokens and n-grams:         " << parameters.threshold << std::endl
            << "- statistical confidence threshold (alpha): " << parameters.alpha << std::endl
            << "- batch size for one thread portion:        " << parameters.batch_size << std::endl
            << "- number of threads:                        " << parameters.num_threads << std::endl
            << "- usage of data cache:                      " << parameters.use_cache << std::endl;

  std::cout << std::endl << "================================================" << std::endl;
  std::cout << "Expected output: " << std::endl;

  if (parameters.output_path.empty()) {
    std::cout << "- no documents will be transformed (due to empty output path)" << std::endl;
  } else {
    if (parameters.return_indices) {
      std::cout << "- documents will be transformed into n-grams indices view, output file: "
                << parameters.output_path << std::endl;
    } else {
      std::cout << "- documents will be transformed into source n-grams view, output file:  "
                << parameters.output_path << std::endl;
    }
  }

  std::cout << "- collocations with their df counters will be stored into file:        "
            << parameters.collocations_output_path << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
  // read and check parameters
  Parameters parameters;
  bool is_help_call = parse_parameters(argc, argv, &parameters);
  if (is_help_call) {
    return 0;
  }

  check_parameters(parameters);
  print_parameters(parameters);

  TopmineImpl::run_topmine(parameters);
}
