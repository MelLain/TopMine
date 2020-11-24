// Author: Murat Apishev (@mel-lain)

#pragma once

#include <atomic>

#include "boost/utility.hpp"

class SpinLock : boost::noncopyable {
 public:
  SpinLock() : state_(kUnlocked) { }
  void lock();
  void unlock();

 private:
  static const bool kLocked = true;
  static const bool kUnlocked = false;
  std::atomic<bool> state_;
};
