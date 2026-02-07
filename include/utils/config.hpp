#pragma once
#include <cstdint>
#include <algorithm>

namespace trading {

using Price = int64_t;

// Order book
constexpr int MAX_SIZE = 256;
constexpr int HALF_SIZE = MAX_SIZE / 2;

// Price limits
constexpr Price MIN_PRICE = 1;
constexpr Price MAX_PRICE = HALF_SIZE - 1;

inline Price clamp_price(Price price) {
    return std::clamp(price, MIN_PRICE, MAX_PRICE);
}

} // namespace trading
