// Author: Murat Apishev (@mel-lain)

#include <string>

#include "boost/functional/hash.hpp"

#include "include/heap.h"

bool IndicesPair::operator==(const IndicesPair& indices) const {
  return indices.token_index == token_index && indices.position_index == position_index;
}

IndicesPair& IndicesPair::operator=(const IndicesPair& indices) {
  if (this != &indices) {
    const_cast<int&>(token_index) = indices.token_index;
    const_cast<int&>(position_index) = indices.position_index;
    const_cast<size_t&>(hash_) = indices.hash_;
  }

  return *this;
}

size_t IndicesPair::caclulate_hash(int token_index, int position_index) {
  size_t hash = 0;
  boost::hash_combine<int>(hash, token_index);
  boost::hash_combine<int>(hash, position_index);
  return hash;
}

bool HeapElement::operator<(const HeapElement& element) const {
  return value < element.value;
}

void Heap::push(const HeapElement& element) {
  push_in_map(&first_to_second_index_, element);
  push_in_map(&second_to_first_index_, element, true);

  elements_[element.indices_first.position_index] = std::make_shared<HeapElement>(element);

  heap_.push(element);
}

HeapElement Heap::pop() {
  if (empty()) {
    throw std::runtime_error("Error: attempt to pop from empty heap");
  }

  HeapElement element;
  while (true) {
    element = heap_.top();
    heap_.pop();

    auto iter = first_to_second_index_.find(element.indices_first);
    if (iter != first_to_second_index_.end()) {
      if (iter->second == element.indices_second) {
        break;
      }
    }
  }

  erase(element);

  return element;
}

void Heap::erase(const HeapElement& element) {
  first_to_second_index_.erase(element.indices_first);
  second_to_first_index_.erase(element.indices_second);
  elements_[element.indices_first.position_index] = nullptr;
}

std::shared_ptr<HeapElement> Heap::get_left_neighbour(const HeapElement& element) const {
  auto iter = second_to_first_index_.find(element.indices_first);
  return (iter == second_to_first_index_.end()) ? nullptr : elements_[iter->second.position_index];
}

std::shared_ptr<HeapElement> Heap::get_right_neighbour(const HeapElement& element) const {
  auto iter = first_to_second_index_.find(element.indices_second);
  return (iter == first_to_second_index_.end()) ? nullptr : elements_[iter->first.position_index];
}

void Heap::clear() {
  // https://stackoverflow.com/questions/2852140/priority-queue-clear-method
  heap_ = std::priority_queue<HeapElement, std::vector<HeapElement>>();

  first_to_second_index_.clear();
  second_to_first_index_.clear();

  elements_.clear();
}

void Heap::push_in_map(std::unordered_map<IndicesPair, IndicesPair, IndicesPairHasher>* map,
                       const HeapElement& element,
                       bool reverse_indices)
{
  auto indices_first = reverse_indices ? element.indices_second : element.indices_first;
  auto indices_second = reverse_indices ? element.indices_first : element.indices_second;

  auto iter = map->find(indices_first);
  if (iter == map->end()) {
    map->emplace(indices_first, indices_second);
    return;
  }

  throw std::runtime_error("Error: attempt to add existing index " +
                           std::to_string(indices_first.token_index) + "_" +
                           std::to_string(indices_first.position_index) +
                           " (reverse_indices is " + std::to_string(reverse_indices) + ")");
}
