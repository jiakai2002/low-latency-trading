#include "../../include/core/order_book.hpp"
#include <algorithm>
#include <limits>
#include <cassert>

namespace trading {

OrderBook::OrderBook() {
    levels_.fill(0);
    best_bid_ = std::nullopt;
    best_ask_ = std::nullopt;
}

void OrderBook::update_best_prices() {
    best_bid_ = std::nullopt;
    best_ask_ = std::nullopt;

    for (int i = 0; i < MAX_SIZE; ++i) {
        if (levels_[i] == 0) continue;
        int norm_price = i - HALF_SIZE;

        if (norm_price < 0) { // bid
            if (!best_bid_ || norm_price < *best_bid_)
                best_bid_ = norm_price;
        } else { // ask
            if (!best_ask_ || norm_price < *best_ask_)
                best_ask_ = norm_price;
        }
    }
}

void OrderBook::add_order(const Order& order) {
    // orders_[order.id] = order;
    orders_.emplace(order.id, order);

    int norm_price = (order.side == Side::Bid ? -order.price : order.price);
    int idx = price_to_index(norm_price);

    // update level
    levels_[idx] += order.quantity;

    // update best price
    if (norm_price < 0) {
        if (!best_bid_ || norm_price < *best_bid_) best_bid_ = norm_price;
    } else {
        if (!best_ask_ || norm_price < *best_ask_) best_ask_ = norm_price;
    }
}

bool OrderBook::cancel_order(OrderId id) {
    auto it = orders_.find(id);
    if (it == orders_.end()) return false;

    const Order& order = it->second;
    int norm_price = (order.side == Side::Bid ? -order.price : order.price);
    int idx = price_to_index(norm_price);

    levels_[idx] -= order.quantity;
    orders_.erase(it);

    // If this level became empty and it was the best price, recompute
    if (levels_[idx] == 0) {
        if ((best_bid_ && norm_price == *best_bid_) ||
            (best_ask_ && norm_price == *best_ask_)) {
            update_best_prices();
        }
    }

    return true;
}

bool OrderBook::modify_order(OrderId id, Quantity newQuantity) {
    auto it = orders_.find(id);
    if (it == orders_.end()) return false;

    if (newQuantity == 0) {
        return cancel_order(id);
    }

    Order& o = it->second;

    int norm_price = (o.side == Side::Bid ? -o.price : o.price);
    assert(std::abs(norm_price) < HALF_SIZE);

    int idx = price_to_index(norm_price);

    Quantity delta = newQuantity - o.quantity;
    levels_[idx] += delta;
    assert(levels_[idx] >= 0);

    o.quantity = newQuantity;

    if (levels_[idx] == 0) {
        if ((best_bid_ && norm_price == *best_bid_) ||
            (best_ask_ && norm_price == *best_ask_)) {
            update_best_prices();
        }
    }

    return true;
}

bool OrderBook::execute_order(OrderId id, Quantity execQuantity) {
    auto it = orders_.find(id);
    if (it == orders_.end()) return false;

    Order& o = it->second;

    Quantity traded = std::min(execQuantity, o.quantity);

    int norm_price = (o.side == Side::Bid ? -o.price : o.price);
    int idx = price_to_index(norm_price);

    // Subtract executed quantity
    if (levels_[idx] <= traded)
        levels_[idx] = 0;
    else
        levels_[idx] -= traded;

    o.quantity -= traded;

    // Remove order if fully executed
    if (o.quantity == 0)
        orders_.erase(it);

    // If this level became empty and it was best price, recompute
    if (levels_[idx] == 0) {
        if ((best_bid_ && norm_price == *best_bid_) ||
            (best_ask_ && norm_price == *best_ask_)) {
            update_best_prices();
        }
    }

    return true;
}

} // namespace trading
