// Author: Murat Apishev (@mel-lain)

#pragma once

#include <atomic>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "boost/thread.hpp"
#include "boost/utility.hpp"

#include "include/batch_processor.h"
#include "include/spinlock.h"

class CollectionProcessorThread : boost::noncopyable {
 public:
  CollectionProcessorThread(BatchProcessor* batch_processor,
                            std::ifstream* input_stream,
                            std::ofstream* output_stream,
                            SpinLock* read_access_lock,
                            SpinLock* write_access_lock,
                            std::vector<std::shared_ptr<Batch>>* data_cache,
                            long* cache_top_index,
                            const std::string& delimiters,
                            int batch_size,
                            bool use_cache)
      : batch_processor_(batch_processor)
      , input_stream_(input_stream)
      , output_stream_(output_stream)
      , read_access_lock_(read_access_lock)
      , write_access_lock_(write_access_lock)
      , data_cache_(data_cache)
      , cache_top_index_(cache_top_index)
      , delimiters_(delimiters)
      , batch_size_(batch_size)
      , use_cache_(use_cache)
      , is_stopping_(false)
      , thread_()
  {
    boost::thread t(&CollectionProcessorThread::thread_function, this);
    thread_.swap(t);
  }

  bool is_stopping() {
    return is_stopping_;
  }

  ~CollectionProcessorThread() {
    is_stopping_ = true;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  void thread_function();

  BatchProcessor* batch_processor_;
  std::ifstream* input_stream_;
  std::ofstream* output_stream_;
  SpinLock* read_access_lock_;
  SpinLock* write_access_lock_;
  std::vector<std::shared_ptr<Batch>>* data_cache_;
  long* cache_top_index_;
  const std::string& delimiters_;
  int batch_size_;
  bool use_cache_;

  mutable std::atomic<bool> is_stopping_;
  boost::thread thread_;
};

class CollectionProcessor {
 public:
  CollectionProcessor(const std::string& input_path,
                      const std::shared_ptr<std::string>& output_path,
                      const std::string& delimiters,
                      int batch_size,
                      bool use_cache)
      : input_path_(input_path)
      , output_path_(output_path)
      , delimiters_(delimiters)
      , batch_size_(batch_size)
      , use_cache_(use_cache)
      , data_cache_()
      , read_access_lock_()
      , write_access_lock_() { }

  void process(const std::vector<BatchProcessor*>& batch_processors);

 private:
  std::string input_path_;
  std::shared_ptr<std::string> output_path_;
  std::string delimiters_;
  int batch_size_;
  bool use_cache_;
  // ToDo(mel-lain): optimize cache by using indices instead of strings
  std::vector<std::shared_ptr<Batch>> data_cache_;
  mutable SpinLock read_access_lock_;
  mutable SpinLock write_access_lock_;
};
