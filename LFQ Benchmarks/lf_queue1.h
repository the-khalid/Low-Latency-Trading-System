#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include <pthread.h>

#include "macros.h"

namespace Common {
  template<typename T>
  class LFQueue1 final { 
  public:
    explicit LFQueue1(std::size_t num_elems) :
        store_(num_elems) {
    }

    auto getNextToWriteTo() noexcept {
      return &store_[next_write_index_];
    }

    auto updateWriteIndex() noexcept {
      next_write_index_ = (next_write_index_ + 1) % store_.size();
      num_elements_++;
    }

    auto getNextToRead() noexcept -> T * { 
      return (size() ? &store_[next_read_index_] : nullptr);
    }

    auto updateReadIndex() noexcept {
      next_read_index_ = (next_read_index_ + 1) % store_.size();
      ASSERT(num_elements_ != 0, "Read an invalid element in:" + std::to_string(pthread_self()));
      num_elements_--;
    }

    auto size() const noexcept {
      return num_elements_.load();
    }
    
    auto capacity() const noexcept {
        return store_.size();
    }

    LFQueue1() = delete;
    LFQueue1(const LFQueue1 &) = delete;
    LFQueue1(const LFQueue1 &&) = delete;
    LFQueue1 &operator=(const LFQueue1 &) = delete;
    LFQueue1 &operator=(const LFQueue1 &&) = delete;

  private:
    std::vector<T> store_;
    std::atomic<size_t> next_write_index_ = {0};
    std::atomic<size_t> next_read_index_ = {0};
    std::atomic<size_t> num_elements_ = {0};
  };
}
