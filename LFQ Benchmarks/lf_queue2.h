#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <new>

#include "macros.h"

namespace Common {
    template<typename T, typename Alloc = std::allocator<T>>
    class LFQueue2 final : private Alloc { 
        std::size_t capacity_;
        T* ring_;
        alignas(64) std::atomic<std::size_t> pushCursor_ {0};
        alignas(64) std::atomic<std::size_t> popCursor_ {0};

        static_assert(std::atomic<std::size_t>::is_always_lock_free);

    public:
        explicit LFQueue2(std::size_t capacity, Alloc const& alloc = Alloc{})
            :   Alloc{alloc}, capacity_{capacity}, ring_{std::allocator_traits<Alloc>::allocate(*this, capacity)}
        {}

        ~LFQueue2() {
            while(not empty()) {
                ring_[popCursor_ % capacity_].~T();
                ++popCursor_;
            }
            std::allocator_traits<Alloc>::deallocate(*this, ring_, capacity_);
        }

        auto capacity() const { return capacity_; }
        auto size() const {
            auto push = pushCursor_.load(std::memory_order_acquire);
            auto pop = popCursor_.load(std::memory_order_acquire);
            return push - pop;
        }
        auto empty() const { return size() == 0; }
        auto full() const { return size() == capacity(); }

        auto push(T const& value) -> bool {
            auto push = pushCursor_.load(std::memory_order_relaxed);
            auto pop = popCursor_.load(std::memory_order_acquire);

            if(push - pop >= capacity_) {
                return false;
            }

            new (&ring_[push % capacity_]) T(value);
            pushCursor_.store(push + 1, std::memory_order_release);
            return true;
        }
        
        auto pop(T& value) -> bool {
            auto push = pushCursor_.load(std::memory_order_acquire);
            auto pop = popCursor_.load(std::memory_order_relaxed);

            if(push == pop) {
                return false;
            }

            value = std::move(ring_[pop % capacity_]); 
            ring_[pop % capacity_].~T();
            popCursor_.store(pop + 1, std::memory_order_release);
            return true;
        }

        LFQueue2() = delete;
        LFQueue2(const LFQueue2 &) = delete;
        LFQueue2(const LFQueue2 &&) = delete;
        LFQueue2 &operator=(const LFQueue2 &) = delete;
        LFQueue2 &operator=(const LFQueue2 &&) = delete;
    };
}
