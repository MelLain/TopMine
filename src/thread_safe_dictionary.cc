// Author: Murat Apishev (@mel-lain)

#include "boost/thread/locks.hpp"

#include "include/thread_safe_dictionary.h"

const int* ThreadSafeDictionary::get_index(const std::string& token) const {
  boost::lock_guard<SpinLock> guard(lock_);
  return get_index_unsafe(token);
}

const std::string* ThreadSafeDictionary::get_token(int index) const {
  boost::lock_guard<SpinLock> guard(lock_);
  return get_token_unsafe(index);
}

const int* ThreadSafeDictionary::get_index_unsafe(const std::string& token) const {
    auto iter = token_to_index_.find(token);
    if (iter != token_to_index_.end()) {
        return &(iter->second);
    }

    return nullptr;
}

const std::string* ThreadSafeDictionary::get_token_unsafe(int index) const {
    if (index < tokens_.size()) {
        return &(tokens_[index]);
    }

    return nullptr;
}

void ThreadSafeDictionary::add(const std::string& token) {
  boost::lock_guard<SpinLock> guard(lock_);
  auto iter = token_to_index_.find(token);
  if (iter == token_to_index_.end()) {
    token_to_index_.emplace(std::make_pair(token, tokens_.size()));
    tokens_.push_back(token);
  }
}

size_t ThreadSafeDictionary::size() const {
  boost::lock_guard<SpinLock> guard(lock_);
  return token_to_index_.size();
}

bool ThreadSafeDictionary::empty() const {
  boost::lock_guard<SpinLock> guard(lock_);
  return token_to_index_.empty();
}
