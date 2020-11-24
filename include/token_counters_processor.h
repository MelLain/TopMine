// Author: Murat Apishev (@mel-lain)

#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <string>

#include "include/batch.h"
#include "include/batch_processor.h"
#include "include/thread_safe_collocation_start_indices.h"
#include "include/thread_safe_dictionary.h"
#include "include/thread_safe_counters.h"

class TokenCountersProcessor : public BatchProcessor {
 public:
  TokenCountersProcessor(const std::shared_ptr<ThreadSafeDictionary>& dictionary,
                         const std::shared_ptr<ThreadSafeCounters>& index_to_counter,
                         const std::shared_ptr<ThreadSafeCollocationStartIndices>& collocation_start_indices,
                         const std::shared_ptr<std::atomic<long>>& total_collection_size)
      : dictionary_(dictionary)
      , index_to_counter_(index_to_counter)
      , collocation_start_indices_(collocation_start_indices)
      , total_collection_size_(total_collection_size) { }

  virtual std::shared_ptr<Batch> process(const Batch& batch);

  virtual ~TokenCountersProcessor() { }

 private:
  std::shared_ptr<ThreadSafeDictionary> dictionary_;
  std::shared_ptr<ThreadSafeCounters> index_to_counter_;
  std::shared_ptr<ThreadSafeCollocationStartIndices> collocation_start_indices_;
  std::shared_ptr<std::atomic<long>> total_collection_size_;
};
