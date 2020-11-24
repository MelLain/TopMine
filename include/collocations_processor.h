// Author: Murat Apishev (@mel-lain)

#pragma once

#include <memory>
#include <string>

#include "include/batch.h"
#include "include/batch_processor.h"
#include "include/thread_safe_collocation_start_indices.h"
#include "include/thread_safe_dictionary.h"
#include "include/thread_safe_counters.h"

class CollocationsProcessor : public BatchProcessor {
 public:
  CollocationsProcessor(const std::shared_ptr<ThreadSafeDictionary>& dictionary,
                        const std::shared_ptr<ThreadSafeCounters>& index_to_counter,
                        const std::shared_ptr<ThreadSafeCollocationStartIndices>& collocation_start_indices,
                        long threshold,
                        char esc_character)
      : dictionary_(dictionary)
      , index_to_counter_(index_to_counter)
      , collocation_start_indices_(collocation_start_indices)
      , collocation_size_(0)
      , threshold_(threshold)
      , esc_character_(esc_character) { }

  virtual std::shared_ptr<Batch> process(const Batch& batch);

  virtual ~CollocationsProcessor() { }

  void set_collocation_size(int collocation_size) {
    collocation_size_ = collocation_size;
  }

 private:
  std::shared_ptr<ThreadSafeDictionary> dictionary_;
  std::shared_ptr<ThreadSafeCounters> index_to_counter_;
  std::shared_ptr<ThreadSafeCollocationStartIndices> collocation_start_indices_;
  int collocation_size_;
  long threshold_;
  char esc_character_;
};
