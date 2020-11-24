// Author: Murat Apishev (@mel-lain)

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/collection_processor.h"
#include "include/heap.h"
#include "include/thread_safe_collocation_start_indices.h"
#include "include/thread_safe_counters.h"
#include "include/thread_safe_dictionary.h"
#include "include/utils.h"

#include "include/token_counters_processor.h"
#include "include/collocations_processor.h"
#include "include/scoring_processor.h"

#include "include/topmine_impl.h"

namespace {
  void store_collocations(const std::string& collocations_output_path,
                          const std::shared_ptr<ThreadSafeCounters>& index_to_counter,
                          const std::shared_ptr<ThreadSafeDictionary>& dictionary)
  {
    std::ofstream output_stream;
    try {
      if (!collocations_output_path.empty()) {
        output_stream.open(collocations_output_path);

        for (const auto& index_counter : index_to_counter->get_all_unsafe()) {
          output_stream << *(dictionary->get_token(index_counter.first)) << " "
                        << index_counter.second << std::endl;
        }

        output_stream.close();
      }
    } catch (std::exception& e) {
      if (output_stream.is_open()) {
        output_stream.close();
      }
    }
  }

  void print_elapsed_time(const std::chrono::time_point<std::chrono::system_clock>& time_start,
                          const std::chrono::time_point<std::chrono::system_clock>& time_end)
  {
    std::cout << "Finish! Elapsed time: "
              << std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start).count()
              << " sec." << std::endl << std::endl;
  }
}  // namespace

void TopmineImpl::run_topmine(const Parameters& parameters) {
  auto time_start = std::chrono::system_clock::now();

  // declare shared data variables
  auto dictionary = std::shared_ptr<ThreadSafeDictionary>(new ThreadSafeDictionary());
  auto index_to_counter = std::shared_ptr<ThreadSafeCounters>(new ThreadSafeCounters());
  auto collocation_index_to_counter = std::shared_ptr<ThreadSafeCounters>(new ThreadSafeCounters());

  auto collocation_start_indices =
    std::shared_ptr<ThreadSafeCollocationStartIndices>(new ThreadSafeCollocationStartIndices());

  auto total_collection_size = std::make_shared<std::atomic<long>>(0L);

  // create smart pointers and raw ones for polymorphism
  std::vector<std::shared_ptr<TokenCountersProcessor>> token_counters_processors;
  std::vector<BatchProcessor*> token_counters_processors_ptr;

  std::vector<std::shared_ptr<CollocationsProcessor>> collocations_processors;
  std::vector<BatchProcessor*> collocations_processors_ptr;

  std::vector<std::shared_ptr<ScoringProcessor>> scoring_processors;
  std::vector<BatchProcessor*> scoring_processors_ptr;

  auto output_path = parameters.output_path.empty() ? nullptr : std::make_shared<std::string>(parameters.output_path);

  for (int thread_id = 0; thread_id < parameters.num_threads; ++thread_id) {
    token_counters_processors.push_back(std::shared_ptr<TokenCountersProcessor>(
      new TokenCountersProcessor(dictionary,
                                 index_to_counter,
                                 collocation_start_indices,
                                 total_collection_size)));

    collocations_processors.push_back(std::shared_ptr<CollocationsProcessor>(
      new CollocationsProcessor(dictionary,
                                index_to_counter,
                                collocation_start_indices,
                                parameters.threshold,
                                parameters.esc_character)));

    scoring_processors.push_back(std::shared_ptr<ScoringProcessor>(
      new ScoringProcessor(dictionary,
                           index_to_counter,
                           collocation_index_to_counter,
                           total_collection_size,
                           parameters.alpha,
                           parameters.collocation_max_size,
                           output_path != nullptr,
                           parameters.return_indices,
                           parameters.esc_character)));

    token_counters_processors_ptr.push_back(token_counters_processors.back().get());
    collocations_processors_ptr.push_back(collocations_processors.back().get());
    scoring_processors_ptr.push_back(scoring_processors.back().get());
  }

  auto collection_processor = std::shared_ptr<CollectionProcessor>(
    new CollectionProcessor(parameters.input_path,
                            output_path,
                            parameters.delimiters,
                            parameters.batch_size,
                            parameters.use_cache));

  // first stage: collecting counters for collocations
  std::cout << "================================================" << std::endl;
  std::cout << "Run processing of token counters..." << std::endl;

  collection_processor->process(token_counters_processors_ptr);

  auto time_prev = std::chrono::system_clock::now();
  print_elapsed_time(time_start, time_prev);

  std::cout << "Total collection size: " << *total_collection_size << std::endl << std::endl;
  std::cout << "Total dictionary size: " << dictionary->size() << std::endl << std::endl;

  std::cout << "Run processing of collocation counters..." << std::endl;
  for (int collocation_size = 2; collocation_size <= parameters.collocation_max_size; ++collocation_size) {
    for (int thread_id = 0; thread_id < parameters.num_threads; ++thread_id) {
      collocations_processors[thread_id]->set_collocation_size(collocation_size);
    }
    collection_processor->process(collocations_processors_ptr);
  }

  print_elapsed_time(time_prev, std::chrono::system_clock::now());
  time_prev = std::chrono::system_clock::now();

  // second stage: extract collocations with significance scores and transform documents
  std::cout << "Run processing of collocation significance scores and documents transformation..." << std::endl;

  collection_processor->process(scoring_processors_ptr);

  print_elapsed_time(time_prev, std::chrono::system_clock::now());

  std::cout << "Collocation index to counter size: " << collocation_index_to_counter->size() << std::endl << std::endl;

  std::cout << "Run storing of collocations into file..." << std::endl;

  store_collocations(parameters.collocations_output_path, collocation_index_to_counter, dictionary);

  std::cout << std::endl << "TopMine finished collection processing!" << std::endl;
  print_elapsed_time(time_start, std::chrono::system_clock::now());

  std::cout << "Max memory usage: " << Utils::get_peak_memory_usage_kb() / 1024
            << " (Kb if MAC OS or Mb if Linux)" << std::endl << std::endl;
  std::cout << "================================================" << std::endl;
}
