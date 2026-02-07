#include "../include/utils/config.hpp"
#include "../include/core/order_book.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>

using namespace trading;

// Benchmark parameters
constexpr size_t NUM_ITERATIONS = 100000;
constexpr size_t NUM_WARMUP = 10000;

// Align to cache line to prevent false sharing
alignas(64) uint64_t g_dummy = 0;

// Measure execution time in nanoseconds
template<typename Func>
uint64_t measure_time_ns(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

// Print benchmark results
void print_results(const std::string& name, const std::vector<uint64_t>& times) {
    std::vector<uint64_t> filtered_times(times.begin() + NUM_WARMUP, times.end());

    if (filtered_times.empty()) {
        std::cout << "No data for " << name << std::endl;
        return;
    }

    std::sort(filtered_times.begin(), filtered_times.end());

    double mean = std::accumulate(filtered_times.begin(), filtered_times.end(), 0.0) / filtered_times.size();

    uint64_t p50 = filtered_times[filtered_times.size() * 50 / 100];
    uint64_t p90 = filtered_times[filtered_times.size() * 90 / 100];
    uint64_t p99 = filtered_times[filtered_times.size() * 99 / 100];
    uint64_t p999 = filtered_times[filtered_times.size() * 999 / 1000];

    std::cout << name << std::endl;
    std::cout << "  Iterations: " << filtered_times.size() << std::endl;
    std::cout << "  mean:     " << std::setw(10) << std::fixed << std::setprecision(2) << mean << " ns" << std::endl;
    std::cout << "  p50:     " << std::setw(10) << p50 << " ns" << std::endl;
    std::cout << "  p90:     " << std::setw(10) << p90 << " ns" << std::endl;
    std::cout << "  p99:     " << std::setw(10) << p99 << " ns" << std::endl;
    std::cout << "  p99.9:   " << std::setw(10) << p999 << " ns" << std::endl;
    std::cout << std::endl;
}

// Benchmark OrderBook operations
void benchmark_order_book() {
    std::cout << "Benchmarking trading::OrderBook..." << std::endl;

    OrderBook ob;

    // Random number generators
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> price_dist(70000, 90000);  // Price between 70.00 and 90.00
    std::uniform_int_distribution<int64_t> quantity_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);

    // Pre-generate orders
    std::vector<Order> orders;
    orders.reserve(NUM_ITERATIONS);
    for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
        Price raw_price = price_dist(gen);
        orders.emplace_back(Order{
            i + 1,
            side_dist(gen) == 0 ? Side::Bid : Side::Ask,
            clamp_price(raw_price),
            quantity_dist(gen)
        });
    }

    // 1. add_order
    {
        std::vector<uint64_t> times;
        times.reserve(NUM_ITERATIONS);
        for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
            times.push_back(measure_time_ns([&]() { ob.add_order(orders[i]); }));
        }
        print_results("add_order", times);
    }

    // 2. get_best_bid / get_best_ask
    {
        std::vector<uint64_t> times;
        times.reserve(NUM_ITERATIONS);
        for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
            times.push_back(measure_time_ns([&]() {
                auto bid = ob.get_best_bid();
                auto ask = ob.get_best_ask();
                g_dummy = bid.value_or(0) + ask.value_or(0);
            }));
        }
        print_results("get_best_bid/get_best_ask", times);
    }

    // 3. modify_order (increase quantity)
    {
        std::vector<uint64_t> times;
        times.reserve(NUM_ITERATIONS);
        for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
            times.push_back(measure_time_ns([&]() {
                ob.modify_order(i + 1, orders[i].quantity + 10);
            }));
        }
        print_results("modify_order", times);
    }

    // 4. cancel_order
    {
        std::vector<uint64_t> times;
        times.reserve(NUM_ITERATIONS);
        for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
            times.push_back(measure_time_ns([&]() { ob.cancel_order(i + 1); }));
        }
        print_results("cancel_order", times);
    }
}

int main() {
    benchmark_order_book();
    return 0;
}
