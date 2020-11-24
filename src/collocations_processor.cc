// Author: Murat Apishev (@mel-lain)

#include <unordered_map>
#include <unordered_set>

#include "boost/range/algorithm_ext/push_back.hpp"
#include "boost/range/irange.hpp"

#include "include/collocations_processor.h"
#include "include/utils.h"

std::shared_ptr<Batch> CollocationsProcessor::process(const Batch& batch) {
  std::unordered_map<int, double> index_to_counter_local;

  for (const auto& document : batch.get_documents()) {
    std::vector<int> next_indices;
    std::unordered_set<int> next_indices_set;

    std::vector<int> indices;
    if (collocation_size_ == 2) {  // special case on first launch
      const auto& document_length = collocation_start_indices_->get_indices(document.id);

      if (document_length.empty()) {
        continue;
      }

      boost::push_back(indices, boost::irange(0, document_length[0]));
    } else {
      indices = collocation_start_indices_->get_indices(document.id);
    }

    for (const auto& index : indices) {
      if (index + collocation_size_ - 2 >= document.tokens.size()) {
        continue;
      }

      auto collocation = Utils::join_strings(document.tokens, index, index + collocation_size_ - 1, esc_character_);

      const int* collocation_index_ptr = dictionary_->get_index(collocation);
      if (collocation_index_ptr != nullptr) {
        const auto counter_ptr = index_to_counter_->get(*collocation_index_ptr);
        if (counter_ptr != nullptr && *counter_ptr >= threshold_) {
          next_indices.push_back(index);
          next_indices_set.insert(index);
        }
      }
    }

    collocation_start_indices_->add_indices(document.id, next_indices);
    if (next_indices.empty()) {
      continue;
    }

    for (const auto& index : next_indices) {
      auto iter = next_indices_set.find(index + 1);
      if (iter == next_indices_set.end()) {
        continue;
      }

      auto collocation = Utils::join_strings(document.tokens, index, index + collocation_size_, esc_character_);

      dictionary_->add(collocation);
      const int* collocation_index_ptr = dictionary_->get_index(collocation);

      ++index_to_counter_local[*collocation_index_ptr];
    }
  }

  index_to_counter_->increase(index_to_counter_local);

  return nullptr;
}
