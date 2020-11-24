// Author: Murat Apishev (@mel-lain)

#pragma once

#include <memory>
#include <queue>
#include <vector>
#include <unordered_map>

#include "boost/thread/locks.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/utility.hpp"

struct IndicesPair {
  IndicesPair() : token_index(), position_index(), hash_() { }

  IndicesPair(int _token_index, int _position_index)
      : token_index(_token_index)
      , position_index(_position_index)
      , hash_(caclulate_hash(_token_index, _position_index)) { }

  bool operator==(const IndicesPair& indices) const;
  IndicesPair& operator=(const IndicesPair& indices);

  size_t hash() const { return hash_; }

  const int token_index;
  const int position_index;

 private:
  static size_t caclulate_hash(int token_index, int position_index);

  const size_t hash_;
};

struct IndicesPairHasher {
  size_t operator()(const IndicesPair& indices) const {
    return indices.hash();
  }
};

struct HeapElement {
  HeapElement()
      : indices_first()
      , indices_second()
      , collocation_size_first()
      , collocation_size_second()
      , value() { }

  HeapElement(const IndicesPair& _indices_first,
              const IndicesPair& _indices_second,
              int _collocation_size_first,
              int _collocation_size_second,
              double _value)
      : indices_first(_indices_first)
      , indices_second(_indices_second)
      , collocation_size_first(_collocation_size_first)
      , collocation_size_second(_collocation_size_second)
      , value(_value) { }

  HeapElement& operator=(const HeapElement& element) = default;
  bool operator<(const HeapElement& element) const;

  IndicesPair indices_first;
  IndicesPair indices_second;

  int collocation_size_first;
  int collocation_size_second;

  double value;
};

class Heap {
 public:
  explicit Heap(int num_elements)
      : heap_()
      , first_to_second_index_()
      , second_to_first_index_()
      , elements_(std::vector<std::shared_ptr<HeapElement>>(num_elements, nullptr)) { }

  void push(const HeapElement& element);
  HeapElement pop();

  void erase(const HeapElement& element);

  std::shared_ptr<HeapElement> get_left_neighbour(const HeapElement& element) const;
  std::shared_ptr<HeapElement> get_right_neighbour(const HeapElement& element) const;

  void clear();

  size_t size() const {
    return heap_.size();
  }

  bool empty() const {
    return first_to_second_index_.empty();
  }

 private:
  static void push_in_map(std::unordered_map<IndicesPair, IndicesPair, IndicesPairHasher>* map,
                          const HeapElement& element,
                          bool reverse_indices = false);

  std::priority_queue<HeapElement, std::vector<HeapElement>> heap_;
  std::unordered_map<IndicesPair, IndicesPair, IndicesPairHasher> first_to_second_index_;
  std::unordered_map<IndicesPair, IndicesPair, IndicesPairHasher> second_to_first_index_;
  std::vector<std::shared_ptr<HeapElement>> elements_;
};
