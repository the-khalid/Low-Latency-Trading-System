#pragma once

#include <string>
#include <random>
#include <algorithm>
#include <chrono>

inline std::string random_string(size_t length) {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string str(length, 0);
    std::generate_n(str.begin(), length, [&]() { return charset[dist(rng)]; });
    return str;
}

class Timer {
public:
    void start() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    double elapsed_s() {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end_time - start_time_;
        return diff.count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};