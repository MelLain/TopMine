// Author: Murat Apishev (@mel-lain)

#include <string>

#include "boost/thread/locks.hpp"

#include "include/thread_safe_collocation_start_indices.h"

void ThreadSafeCollocationStartIndices::add_indices(long document_id, const std::vector<int>& indices) {
  boost::lock_guard<SpinLock> guard(lock_);
  indices_[document_id] = indices;
}

const std::vector<int>& ThreadSafeCollocationStartIndices::get_indices(long document_id) const {
  boost::lock_guard<SpinLock> guard(lock_);

  auto document_iter = indices_.find(document_id);

  if (document_iter != indices_.end()) {
    return document_iter->second;
  }

  throw std::runtime_error("Error: no indices for id " + std::to_string(document_id));
}

size_t ThreadSafeCollocationStartIndices::size() const {
  boost::lock_guard<SpinLock> guard(lock_);
  return indices_.size();
}
