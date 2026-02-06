#include "order_book.hpp"
#include <chrono>
#include <random>
#include <vector>
#include <numeric>
#include <iostream>
#include <iomanip>

using namespace trading;
using namespace std::chrono;

constexpr size_t NUM_ITERATIONS = 120000;

// --- Timing helper ---
template <typename Func>
uint64_t measure_time_ns(Func&& func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    return duration_cast<nanoseconds>(end - start).count();
}

// --- Percentile helper ---
long long pct(std::vector<long long> v, double p) {
    std::sort(v.begin(), v.end());
    size_t i = std::min(v.size() - 1, static_cast<size_t>(p / 100.0 * v.size()));
    return v[i];
}

// --- Mean helper ---
double mean(const std::vector<long long>& v) {
    return std::accumulate(v.begin(), v.end(), 0LL) / static_cast<double>(v.size());
}

// --- Print results ---
void print_results(const char* name, const std::vector<long long>& times) {
    if (times.empty()) return;
    std::cout << "\n" << name << " (ns)\n";
    std::cout << std::left << std::setw(12) << "mean" << std::fixed << std::setprecision(0) << mean(times) << "\n";
    std::cout << std::left << std::setw(12) << "p50" << pct(times, 50) << "\n";
    std::cout << std::left << std::setw(12) << "p90" << pct(times, 90) << "\n";
    std::cout << std::left << std::setw(12) << "p99" << pct(times, 99) << "\n";
    std::cout << std::left << std::setw(12) << "p99.9" << pct(times, 99.9) << "\n";
}

int main() {
    OrderBook ob;

    std::mt19937 gen(42);
    std::uniform_int_distribution<int> price_dist(50, 250);  // Prices
    std::discrete_distribution<int> qty_dist{50,30,15,4,1};  // 1,5,10,25,50
    int sizes[] = {1,5,10,25,50};
    std::bernoulli_distribution sideFlip(0.5);

    std::vector<Order> orders;
    orders.reserve(NUM_ITERATIONS);
    for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
        orders.push_back({
            i+1,
            sideFlip(gen) ? Side::Bid : Side::Ask,
            price_dist(gen),
            sizes[qty_dist(gen)]
        });
    }

    std::vector<long long> addLat, cancelLat, bestLat;
    std::vector<OrderId> liveIds;

    // --- Benchmark add_order ---
    for (auto& o : orders) {
        auto t = measure_time_ns([&]() {
            ob.add_order(o);
        });
        addLat.push_back(t);
        liveIds.push_back(o.id);
    }

    // --- Benchmark get_best_prices ---
    for (size_t i = 0; i < NUM_ITERATIONS; i += 3) {
        auto t = measure_time_ns([&]() {
            auto [bestBid, bestAsk] = ob.get_best_prices();
            (void)bestBid; 
            (void)bestAsk;
        });
        bestLat.push_back(t);
    }

    // --- Benchmark cancel_order ---
    for (auto id : liveIds) {
        auto t = measure_time_ns([&]() {
            ob.cancel_order(id);
        });
        cancelLat.push_back(t);
    }

    // --- Print statistics ---
    print_results("Add", addLat);
    print_results("BestPrice", bestLat);
    print_results("Cancel", cancelLat);

    return 0;
}
