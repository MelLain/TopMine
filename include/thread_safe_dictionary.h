// Author: Murat Apishev (@mel-lain)

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "boost/utility.hpp"

#include "include/spinlock.h"

class ThreadSafeDictionary : boost::noncopyable {
 public:
  const int* get_index(const std::string& token) const;
  const std::string* get_token(int index) const;

  const int* get_index_unsafe(const std::string& token) const;
  const std::string* get_token_unsafe(int index) const;

  void add(const std::string& token);

  size_t size() const;
  bool empty() const;

 private:
  mutable SpinLock lock_;
  std::unordered_map<std::string, int> token_to_index_;
  std::vector<std::string> tokens_;
};
