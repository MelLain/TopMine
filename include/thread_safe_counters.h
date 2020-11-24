// Author: Murat Apishev (@mel-lain)

#pragma once

#include <vector>
#include <unordered_map>

#include "boost/utility.hpp"

#include "include/spinlock.h"

class ThreadSafeCounters : boost::noncopyable {
 public:
  const double* get(int key) const;

  void increase(int key, double value);
  void increase(const std::unordered_map<int, double>& key_to_value);

  size_t size() const;
  bool empty() const;

  const std::unordered_map<int, double>& get_all_unsafe() const {
    return index_to_counter_;
  }

 private:
  mutable SpinLock lock_;
  std::unordered_map<int, double> index_to_counter_;
};
