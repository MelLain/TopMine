// Author: Murat Apishev (@mel-lain)

#include <unordered_map>

#include "boost/range/algorithm_ext/push_back.hpp"
#include "boost/range/irange.hpp"

#include "include/token_counters_processor.h"

std::shared_ptr<Batch> TokenCountersProcessor::process(const Batch& batch) {
  std::unordered_map<int, double> index_to_counter_local;

  for (const auto& document : batch.get_documents()) {
    collocation_start_indices_->add_indices(
      document.id, std::vector<int>(1, static_cast<int>(document.tokens.size())));

    for (const auto& token : document.tokens) {
      dictionary_->add(token);
      const int* index = dictionary_->get_index(token);

      ++index_to_counter_local[*index];
    }
  }

  double counter = 0.0;
  for (const auto& index_value : index_to_counter_local) {
    counter += index_value.second;
  }

  index_to_counter_->increase(index_to_counter_local);

  *total_collection_size_ += static_cast<long>(counter);

  return nullptr;
}
