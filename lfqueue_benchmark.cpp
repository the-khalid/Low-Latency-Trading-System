#include <iostream>
#include <thread>
#include <vector>

// --- Make sure you save your queue implementations in these files ---
#include "lf_queue1.h" // The first implementation from the book
#include "lf_queue2.h" // Your second, more advanced implementation

#include "benchmark_helpers.h"

constexpr size_t NUM_OPERATIONS = 5'000'000;
constexpr size_t QUEUE_CAPACITY = 1024;

// --- Benchmark for the first LFQueue implementation ---
void benchmark_queue1() {
    Common::LFQueue<std::string> queue(QUEUE_CAPACITY);
    Timer timer;
    
    std::cout << "--- Benchmarking LFQueue #1 (Book's initial design) ---" << std::endl;

    auto producer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string value = random_string(128);
            // This is a busy-wait loop, which is inefficient but simple for a benchmark
            while (queue.size() >= QUEUE_CAPACITY) {} 
            *queue.getNextToWriteTo() = value;
            queue.updateWriteIndex();
        }
    });

    auto consumer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            const std::string* value;
            // Busy-wait until an element is available
            while ((value = queue.getNextToRead()) == nullptr) {}
            // In a real app, you would use the value. Here we just consume it.
            queue.updateReadIndex();
        }
    });

    timer.start();
    producer.join();
    consumer.join();

    double elapsed = timer.elapsed_s();
    std::cout << "LFQueue #1 elapsed time: " << elapsed << " s" << std::endl;
    std::cout << "Operations per second: " << (NUM_OPERATIONS / elapsed) << std::endl;
}

// --- Benchmark for the second LFQueue implementation ---
void benchmark_queue2() {
    Common::LFQueue<std::string> queue(QUEUE_CAPACITY);
    Timer timer;

    std::cout << "\n--- Benchmarking LFQueue #2 (Your custom design) ---" << std::endl;
    
    // IMPORTANT: Fix the memory order bug from your previous code
    // This is a corrected pop implementation
    auto corrected_pop = [&](std::string& value) -> bool {
        auto push = queue.pushCursor_.load(std::memory_order_acquire); // MUST be acquire
        auto pop = queue.popCursor_.load(std::memory_order_relaxed);

        if (push == pop) {
            return false;
        }

        value = std::move(queue.ring_[pop % queue.capacity_]);
        queue.ring_[pop % queue.capacity_].~string();
        queue.popCursor_.store(pop + 1, std::memory_order_release);
        return true;
    };


    auto producer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string value = random_string(128);
            while (!queue.push(value)) {} // Busy-wait
        }
    });

    auto consumer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string value;
            while (!corrected_pop(value)) {} // Busy-wait with corrected logic
        }
    });

    timer.start();
    producer.join();
    consumer.join();

    double elapsed = timer.elapsed_s();
    std::cout << "LFQueue #2 elapsed time: " << elapsed << " s" << std::endl;
    std::cout << "Operations per second: " << (NUM_OPERATIONS / elapsed) << std::endl;
}


int main() {
    benchmark_queue1();
    benchmark_queue2();
    return 0;
}