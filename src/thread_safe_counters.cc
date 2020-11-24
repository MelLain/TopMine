// Author: Murat Apishev (@mel-lain)

#include <string>
#include <utility>

#include "boost/thread/locks.hpp"

#include "include/thread_safe_counters.h"

const double* ThreadSafeCounters::get(int key) const {
  boost::lock_guard<SpinLock> guard(lock_);
  auto iter = index_to_counter_.find(key);
  if (iter != index_to_counter_.end()) {
    return &(iter->second);
  }

  return nullptr;
}

void ThreadSafeCounters::increase(int key, double value) {
  boost::lock_guard<SpinLock> guard(lock_);
  auto iter = index_to_counter_.find(key);
  if (iter != index_to_counter_.end()) {
    iter->second += value;
  } else {
    index_to_counter_.emplace(std::make_pair(key, value));
  }
}

void ThreadSafeCounters::increase(const std::unordered_map<int, double>& key_to_value) {
  boost::lock_guard<SpinLock> guard(lock_);
  for (const auto& key_value : key_to_value) {
    auto iter = index_to_counter_.find(key_value.first);
    if (iter != index_to_counter_.end()) {
      iter->second += key_value.second;
    } else {
      index_to_counter_.emplace(std::make_pair(key_value.first, key_value.second));
    }
  }
}

size_t ThreadSafeCounters::size() const {
  boost::lock_guard<SpinLock> guard(lock_);
  return index_to_counter_.size();
}

bool ThreadSafeCounters::empty() const {
  boost::lock_guard<SpinLock> guard(lock_);
  return index_to_counter_.empty();
}
