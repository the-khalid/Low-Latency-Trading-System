#!/bin/bash

set -e

echo "--- Compiling Benchmark ---"

g++ -std=c++17 -g -O3 -DNDEBUG -pthread lfqueue_benchmark.cpp -o lfqueue_benchmark

echo "--- Running Benchmark without perf (for baseline speed) ---"
./lfqueue_benchmark

echo -e "\n--- Profiling with perf stat (high-level overview) ---"
echo "This command gives you a summary of performance counters."
echo "Watch for high context-switches, cache-misses, and instructions per cycle (IPC)."

perf stat -e cpu-cycles,instructions,cache-references,cache-misses,context-switches,cpu-migrations ./lfqueue_benchmark

echo -e "\n--- Profiling with perf record (in-depth analysis) ---"
echo "This command records performance samples, creating a perf.data file."
echo "Use 'perf report' to analyze the results and see which functions are 'hot'."

perf record -g ./lfqueue_benchmark

echo -e "\n--- Analysis ---"
echo "A 'perf.data' file has been created."
echo "Run 'perf report' to view the interactive analysis."
echo "Inside 'perf report', you can press 'a' to annotate the assembly code for the hottest functions."
echo "This will show you exactly which CPU instructions are causing cache misses or taking the most cycles."
