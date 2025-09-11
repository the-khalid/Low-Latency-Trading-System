#include <iostream>
#include <thread>
#include <vector>
#include <string>

#include "lf_queue1.h" // Renamed class
#include "lf_queue2.h" // Renamed class
#include "benchmark_helpers.h"

constexpr size_t NUM_OPERATIONS = 5'000'000;
constexpr size_t QUEUE_CAPACITY = 1024;

// --- Benchmark for the first LFQueue implementation ---
void benchmark_queue1() {
    Common::LFQueue1<std::string> queue(QUEUE_CAPACITY);
    Timer timer;
    
    std::cout << "--- Benchmarking LFQueue1 (Book's design) ---" << std::endl;

    auto producer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string value = random_string(128);
            while (queue.size() >= queue.capacity()) {} // Busy-wait
            *queue.getNextToWriteTo() = value;
            queue.updateWriteIndex();
        }
    });

    auto consumer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string* value_ptr;
            while ((value_ptr = queue.getNextToRead()) == nullptr) {} // Busy-wait
            std::string value = std::move(*value_ptr); // Consume with a move
            queue.updateReadIndex();
        }
    });

    timer.start();
    producer.join();
    consumer.join();

    double elapsed = timer.elapsed_s();
    std::cout << "LFQueue1 elapsed time: " << elapsed << " s" << std::endl;
    std::cout << "Operations per second: " << (double)NUM_OPERATIONS / elapsed << std::endl;
}

// --- Benchmark for the second LFQueue implementation ---
void benchmark_queue2() {
    Common::LFQueue2<std::string> queue(QUEUE_CAPACITY);
    Timer timer;

    std::cout << "\n--- Benchmarking LFQueue2 (Your custom design) ---" << std::endl;
    
    auto producer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string value = random_string(128);
            while (!queue.push(value)) {} // Busy-wait
        }
    });

    auto consumer = std::thread([&]() {
        for (size_t i = 0; i < NUM_OPERATIONS; ++i) {
            std::string value;
            while (!queue.pop(value)) {} // Busy-wait
        }
    });

    timer.start();
    producer.join();
    consumer.join();

    double elapsed = timer.elapsed_s();
    std::cout << "LFQueue2 elapsed time: " << elapsed << " s" << std::endl;
    std::cout << "Operations per second: " << (double)NUM_OPERATIONS / elapsed << std::endl;
}


int main() {
//    benchmark_queue1();
    benchmark_queue2();
    return 0;
}
