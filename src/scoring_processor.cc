// Author: Murat Apishev (@mel-lain)

#include <cmath>

#include <map>
#include <sstream>

#include "include/common.h"
#include "include/utils.h"

#include "include/scoring_processor.h"

double ScoringProcessor::compute_pair_score(int index_first,
                                            int index_second,
                                            const std::string& token_first,
                                            const std::string& token_second) const
{
  double mu = *(index_to_counter_->get(index_first)) * *(index_to_counter_->get(index_second));
  mu /= static_cast<double>(*total_collection_size_);

  auto collocation = Utils::join_strings({ token_first, token_second }, esc_character_);

  double pair_frequency = 0.0;
  const int* token_index_ptr = dictionary_->get_index_unsafe(collocation);
  if (token_index_ptr != nullptr) {
    pair_frequency = *(index_to_counter_->get(*token_index_ptr));
  }

  return pair_frequency > kEps ? (pair_frequency - mu) / std::sqrt(pair_frequency) : 0.0;
}

void ScoringProcessor::add_processed_item(const std::shared_ptr<Batch>& processed_batch,
                                          const std::unordered_map<int, Collocation>& position_to_collocation,
                                          const Document& document)
{
  std::vector<std::string> tokens;

  for (int i = 0; i < document.tokens.size();) {
    auto str_i = std::to_string(i);
    auto iter = position_to_collocation.find(i);

    if (iter == position_to_collocation.end()) {
      tokens.push_back(return_indices_ ? Utils::join_strings({ str_i, "1" }, esc_character_) : document.tokens[i]);

      ++i;
      continue;
    }

    tokens.push_back(return_indices_ ? Utils::join_strings({ str_i, std::to_string(iter->second.collocation_size) },
                                                           esc_character_)
                                     : *(dictionary_->get_token_unsafe(iter->second.collocation_index)));

    i += iter->second.collocation_size;
  }

  processed_batch->add_document(document.id, tokens);
}

std::shared_ptr<Batch> ScoringProcessor::process(const Batch& batch) {
  std::unordered_map<int, Collocation> position_to_collocation;
  auto processed_batch = std::make_shared<Batch>(Batch(batch.delimiters));

  for (const auto& document : batch.get_documents()) {
    const int num_elements = document.tokens.size() - 1;
    Heap token_pairs_heap(num_elements);

    for (int i = 0; i < num_elements; ++i) {
      int index_first = *(dictionary_->get_index_unsafe(document.tokens[i]));
      int index_second = *(dictionary_->get_index_unsafe(document.tokens[i + 1]));

      double score = compute_pair_score(index_first, index_second, document.tokens[i], document.tokens[i + 1]);

      if (score >= alpha_) {
        token_pairs_heap.push({ { index_first, i }, { index_second, i + 1 }, 1, 1, score });
      }
    }

    while (!token_pairs_heap.empty()) {
      auto element = token_pairs_heap.pop();

      if (element.value < alpha_) {
        position_to_collocation.emplace(element.indices_first.position_index,
          Collocation(element.indices_first.token_index, element.collocation_size_first));

        position_to_collocation.emplace(element.indices_second.position_index,
          Collocation(element.indices_second.token_index, element.collocation_size_second));

        continue;
      }

      auto collocation = Utils::join_strings({ *(dictionary_->get_token_unsafe(element.indices_first.token_index)),
                                               *(dictionary_->get_token_unsafe(element.indices_second.token_index)) },
                                             esc_character_);

      int collocation_index = *(dictionary_->get_index_unsafe(collocation));
      int collocation_size = element.collocation_size_first + element.collocation_size_second;

      auto left_element = token_pairs_heap.get_left_neighbour(element);
      auto right_element = token_pairs_heap.get_right_neighbour(element);

      if ((left_element == nullptr && right_element == nullptr) || collocation_size >= collocation_max_size_) {
        position_to_collocation.emplace(element.indices_first.position_index,
          Collocation(collocation_index, collocation_size));

        continue;
      }

      if (left_element != nullptr) {
        int token_index_left = left_element->indices_first.token_index;

        double score = compute_pair_score(token_index_left,
                                          collocation_index,
                                          *(dictionary_->get_token_unsafe(token_index_left)),
                                          collocation);

        token_pairs_heap.erase(*left_element);

        token_pairs_heap.push({ { token_index_left, left_element->indices_first.position_index },
                                { collocation_index, element.indices_first.position_index },
                                left_element->collocation_size_first,
                                collocation_size,
                                score });
      }

      if (right_element != nullptr) {
        int token_index_right = right_element->indices_second.token_index;

        double score = compute_pair_score(collocation_index,
                                          token_index_right,
                                          collocation,
                                          *(dictionary_->get_token_unsafe(token_index_right)));

        token_pairs_heap.erase(*right_element);

        token_pairs_heap.push({ { collocation_index, element.indices_first.position_index },
                                { token_index_right, right_element->indices_second.position_index },
                                collocation_size,
                                right_element->collocation_size_second,
                                score });
      }
    }

    if (return_processed_batch_) {
      add_processed_item(processed_batch, position_to_collocation, document);
    }

    for (const auto& index_collocation : position_to_collocation) {
      collocation_index_to_counter_->increase(index_collocation.second.collocation_index, 1.0);
    }

    position_to_collocation.clear();
  }

  return return_processed_batch_ ? processed_batch : nullptr;
}
