#pragma once

#include <vector>
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
    Price price = 0;
    Quantity quantity = 0;
};

struct Level {
    Price price = 0;
    Quantity quantity = 0;
};

class OrderBook {
public:
    OrderBook() = default;

    void add_order(const Order& order);
    bool cancel_order(OrderId id);
    bool modify_order(OrderId id, Quantity newQuantity);
    bool execute_order(OrderId id, Quantity execQuantity);

    std::pair<Price, Price> get_best_prices() const;

private:
    std::vector<Level> bidLevels_;
    std::vector<Level> askLevels_;
    std::unordered_map<OrderId, Order> orders_;

    template <typename Compare>
    void add_level(std::vector<Level>& levels, Price price, Quantity quantity, Compare comp);

    template <typename Compare>
    void remove_level(std::vector<Level>& levels, Price price, Quantity quantity, Compare comp);
};

} // namespace trading
