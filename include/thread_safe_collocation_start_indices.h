// Author: Murat Apishev (@mel-lain)

#pragma once

#include <unordered_map>
#include <vector>

#include "boost/utility.hpp"

#include "include/spinlock.h"

class ThreadSafeCollocationStartIndices : boost::noncopyable {
 public:
  void add_indices(long document_id, const std::vector<int>& indices);

  const std::vector<int>& get_indices(long document_id) const;

  size_t size() const;

 private:
  mutable SpinLock lock_;
  std::unordered_map<long, std::vector<int>> indices_;
};
