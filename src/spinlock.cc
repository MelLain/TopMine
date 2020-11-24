// Author: Murat Apishev (@mel-lain)

#include "include/spinlock.h"

void SpinLock::lock() {
  while (state_.exchange(kLocked, std::memory_order_acquire) == kLocked) {
    /* busy-wait */
  }
}

void SpinLock::unlock() {
  state_.store(kUnlocked, std::memory_order_release);
}
