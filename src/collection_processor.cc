// Author: Murat Apishev (@mel-lain)

#include <iostream>
#include <string>

#include "boost/thread/locks.hpp"

#include "include/collection_processor.h"

void CollectionProcessorThread::thread_function() {
  try {
    while (true) {
      std::shared_ptr<Batch> batch;
      if (input_stream_ == nullptr) {
        {
          boost::lock_guard<SpinLock> guard(*read_access_lock_);

          if (*cache_top_index_ >= data_cache_->size()) {
            break;
          }

          batch = (*data_cache_)[(*cache_top_index_)++];
        }
      } else {
        batch.reset(new Batch(delimiters_));
        {
          boost::lock_guard<SpinLock> guard(*read_access_lock_);

          if (input_stream_->eof()) {
            break;
          }

          while (batch->size() < batch_size_) {
            std::string str;
            std::getline(*input_stream_, str);

            if (input_stream_->eof()) {
              break;
            }

            batch->add_document(str);
          }

          if (use_cache_) {
            data_cache_->push_back(batch);
          }
        }
      }

      auto processed_batch = batch_processor_->process(*batch);

      if (processed_batch != nullptr && output_stream_ != nullptr) {
        boost::lock_guard<SpinLock> guard(*write_access_lock_);

        for (const auto& document : processed_batch->get_documents()) {
          (*output_stream_) << document.id;
          for (const auto& token : document.tokens) {
            (*output_stream_) << delimiters_[0] << token;
          }
          (*output_stream_) << std::endl;
        }
      }
    }

    is_stopping_ = true;
  } catch (std::exception& e) {
    is_stopping_ = true;

    std::cerr << e.what() << std::endl;
    throw e;
  }
}

void CollectionProcessor::process(const std::vector<BatchProcessor*>& batch_processors) {
  std::shared_ptr<std::ifstream> input_stream = nullptr;
  std::shared_ptr<std::ofstream> output_stream = nullptr;

  try {
    if (!use_cache_ || data_cache_.empty()) {
      input_stream.reset(new std::ifstream(input_path_));
    }

    if (output_path_ != nullptr) {
      output_stream.reset(new std::ofstream(*output_path_));
    }

    long cache_top_index = 0L;
    std::vector<std::shared_ptr<CollectionProcessorThread>> threads;

    for (const auto& processor : batch_processors) {
      threads.push_back(std::shared_ptr<CollectionProcessorThread>(
        new CollectionProcessorThread(processor,
                                      input_stream.get(),
                                      output_stream.get(),
                                      &read_access_lock_,
                                      &write_access_lock_,
                                      &data_cache_,
                                      &cache_top_index,
                                      delimiters_,
                                      batch_size_,
                                      use_cache_)));
    }

    while (true) {
      bool is_finished = true;

      for (const auto& thread : threads) {
        if (!thread->is_stopping()) {
          is_finished = false;
        }
      }

      if (is_finished) {
        break;
      }
      usleep(2000);
    }

    if (output_stream != nullptr) {
      output_stream->close();
    }
  } catch (std::exception& e) {
    if (input_stream->is_open()) {
      input_stream->close();
    }

    if (output_stream != nullptr && output_stream->is_open()) {
      output_stream->close();
    }

    throw e;
  }
}
