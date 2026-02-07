#pragma once
#include "../utils/config.hpp"
#include <array>
#include <unordered_map>
#include <cstdint>
#include <algorithm>

namespace trading {

using Price = int64_t;
using Quantity = int64_t;
using OrderId = uint64_t;

enum class Side { Bid, Ask };

struct Order {
    OrderId id = 0;
    Side side;
    Price price;
    Quantity quantity;
};

class OrderBook {
public:
    OrderBook();

    void add_order(const Order& order);
    bool cancel_order(OrderId id);
    bool modify_order(OrderId id, Quantity new_quantity);
    bool execute_order(OrderId id, Quantity exec_quantity);

    std::optional<Price> get_best_bid() const {
        if (!best_bid_) return std::nullopt;
        return -*best_bid_;
    }

    std::optional<Price> get_best_ask() const { return best_ask_; }

private:
    std::array<Quantity, MAX_SIZE> levels_{};
    std::unordered_map<OrderId, Order> orders_;

    std::optional<Price> best_bid_;
    std::optional<Price> best_ask_;

    int price_to_index(int norm_price) const {
        return (norm_price + HALF_SIZE) & (MAX_SIZE - 1);
    }

    void update_best_prices();
};

} // namespace trading
