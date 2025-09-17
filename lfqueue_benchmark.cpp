#include <iostream>
#include <thread>
#include <vector>

#include "lf_queue1.h"
#include "lf_queue2.h"

#include "benchmark_helpers.h"

constexpr size_t NUM_OPERATIONS = 5'000'000;
constexpr size_t QUEUE_CAPACITY = 1024;

void benchmark_queue1() {
    Common::LFQueue<std::string> queue(QUEUE_CAPACITY);
    Timer timer;
    
    std::cout << "--- Benchmarking LFQueue #1 (Book's initial design) ---" << std::endl;

    auto producer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string value = random_string(128);
            
            while (queue.size() >= QUEUE_CAPACITY) {} 
            *queue.getNextToWriteTo() = value;
            queue.updateWriteIndex();
        }
    });

    auto consumer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            const std::string* value;
            
            while ((value = queue.getNextToRead()) == nullptr) {}
            
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


void benchmark_queue2() {
    Common::LFQueue<std::string> queue(QUEUE_CAPACITY);
    Timer timer;

    std::cout << "\n--- Benchmarking LFQueue #2 (Your custom design) ---" << std::endl;
    
    auto corrected_pop = [&](std::string& value) -> bool {
        auto push = queue.pushCursor_.load(std::memory_order_acquire); 
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
            while (!corrected_pop(value)) {} 
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