#include "order_book.hpp"
#include "config.hpp"
#include <chrono>
#include <random>
#include <vector>
#include <numeric>
#include <iostream>
#include <iomanip>

using namespace trading;
using namespace std::chrono;

// ------------------- Benchmark helpers -------------------

template <typename F>
uint64_t measure_time_ns(F&& func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    return duration_cast<nanoseconds>(end - start).count();
}

double mean(const std::vector<uint64_t>& v) {
    return static_cast<double>(std::accumulate(v.begin(), v.end(), 0ULL)) / v.size();
}

uint64_t pct(std::vector<uint64_t> v, double p) {
    std::sort(v.begin(), v.end());
    size_t i = std::min(v.size() - 1, static_cast<size_t>(p / 100.0 * v.size()));
    return v[i];
}

void print_results(const std::string& name, const std::vector<uint64_t>& times) {
    if (times.empty()) return;

    auto ns_to_us = [](double ns) { return ns / 1000.0; };

    std::cout << "\n" << name << " (µs)\n";
    std::cout << std::left << std::setw(12) << "mean"  << std::fixed << std::setprecision(2) << ns_to_us(mean(times)) << "\n";
    std::cout << std::left << std::setw(12) << "p50"   << ns_to_us(pct(times,50)) << "\n";
    std::cout << std::left << std::setw(12) << "p90"   << ns_to_us(pct(times,90)) << "\n";
    std::cout << std::left << std::setw(12) << "p99"   << ns_to_us(pct(times,99)) << "\n";
    std::cout << std::left << std::setw(12) << "p99.9" << ns_to_us(pct(times,99.9)) << "\n";
}

// ------------------- Benchmark -------------------

int main() {
    constexpr size_t NUM_EVENTS = 100000;

    OrderBook ob;

    std::vector<OrderId> live_ids;
    live_ids.reserve(NUM_EVENTS);

    std::vector<uint64_t> addLat, cancelLat, bestLat;

    // Pre-generate orders to avoid RNG inside timed loop
    std::vector<Order> orders;
    orders.reserve(NUM_EVENTS);

    std::mt19937 gen(42);
    std::uniform_int_distribution<Price> price_dist(0, 300); // wider range, then clamp
    std::uniform_int_distribution<Quantity> qty_dist(1,50);
    std::bernoulli_distribution side_dist(0.5);

    for (size_t i = 0; i < NUM_EVENTS; ++i) {
        Price raw_price = price_dist(gen);
        orders.push_back({
            i+1,
            side_dist(gen) ? Side::Bid : Side::Ask,
            clamp_price(raw_price),
            qty_dist(gen)
        });
    }

    // --- Benchmark add_order ---
    for (const auto& o : orders) {
        uint64_t t = measure_time_ns([&]() {
            ob.add_order(o);
        });
        addLat.push_back(t);
        live_ids.push_back(o.id);
    }

    // --- Benchmark best_bid / best_ask reads ---
    for (size_t i = 0; i < NUM_EVENTS; ++i) {
        uint64_t t = measure_time_ns([&]() {
            auto b = ob.get_best_bid();
            auto a = ob.get_best_ask();
            (void)b; (void)a; // prevent optimization
        });
        bestLat.push_back(t);
    }

    // --- Benchmark cancel_order ---
    for (auto id : live_ids) {
        uint64_t t = measure_time_ns([&]() {
            ob.cancel_order(id);
        });
        cancelLat.push_back(t);
    }

    // --- Print results ---
    print_results("Add Order", addLat);
    print_results("Best Price Read", bestLat);
    print_results("Cancel Order", cancelLat);

    return 0;
}
