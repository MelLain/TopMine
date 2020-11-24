// Author: Murat Apishev (@mel-lain)

#pragma once

#include <memory>

#include "include/batch.h"

class BatchProcessor {
 public:
  virtual std::shared_ptr<Batch> process(const Batch& batch) = 0;

  virtual ~BatchProcessor() { }
};
