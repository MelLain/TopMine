// Author: Murat Apishev (@mel-lain)

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "include/batch.h"
#include "include/batch_processor.h"
#include "include/thread_safe_dictionary.h"
#include "include/thread_safe_counters.h"
#include "include/heap.h"

struct Collocation {
  Collocation(int _collocation_index, int _collocation_size)
      : collocation_index(_collocation_index)
      , collocation_size(_collocation_size) { }

  int collocation_index;
  int collocation_size;
};

class ScoringProcessor : public BatchProcessor {
 public:
  ScoringProcessor(const std::shared_ptr<ThreadSafeDictionary>& dictionary,
                   const std::shared_ptr<ThreadSafeCounters>& index_to_counter,
                   const std::shared_ptr<ThreadSafeCounters>& collocation_index_to_counter,
                   const std::shared_ptr<std::atomic<long>>& total_collection_size,
                   float alpha,
                   int collocation_max_size,
                   bool return_processed_batch,
                   bool return_indices,
                   char esc_character)
      : dictionary_(dictionary)
      , index_to_counter_(index_to_counter)
      , collocation_index_to_counter_(collocation_index_to_counter)
      , total_collection_size_(total_collection_size)
      , alpha_(alpha)
      , collocation_max_size_(collocation_max_size)
      , return_processed_batch_(return_processed_batch)
      , return_indices_(return_indices)
      , esc_character_(esc_character) { }

  virtual std::shared_ptr<Batch> process(const Batch& batch);

  virtual ~ScoringProcessor() { }

 private:
  double compute_pair_score(int index_first,
                            int index_second,
                            const std::string& token_first,
                            const std::string& token_second) const;

  void add_processed_item(const std::shared_ptr<Batch>& processed_batch,
                          const std::unordered_map<int, Collocation>& position_to_collocation,
                          const Document& document);

  std::shared_ptr<ThreadSafeDictionary> dictionary_;
  std::shared_ptr<ThreadSafeCounters> index_to_counter_;
  std::shared_ptr<ThreadSafeCounters> collocation_index_to_counter_;
  std::shared_ptr<std::atomic<long>> total_collection_size_;
  float alpha_;
  int collocation_max_size_;
  bool return_processed_batch_;
  bool return_indices_;
  char esc_character_;
};
