#include "order_book.hpp"
#include <algorithm>
#include <functional>

namespace trading {

inline auto bidComp() { return std::less<Price>{}; }     // highest bid last
inline auto askComp() { return std::greater<Price>{}; } // lowest ask last

// Add / Remove generic levels
template <typename Compare>
void OrderBook::add_level(std::vector<Level>& levels, Price price, Quantity quantity, Compare comp) {
    auto it = std::lower_bound(levels.begin(), levels.end(), price,
        [comp](const Level& l, Price p) { return comp(l.price, p); });

    if (it != levels.end() && it->price == price) {
        it->quantity += quantity;
    } else {
        levels.insert(it, Level{price, quantity});
    }
}

template <typename Compare>
void OrderBook::remove_level(std::vector<Level>& levels, Price price, Quantity quantity, Compare comp) {
    auto it = std::lower_bound(levels.begin(), levels.end(), price,
        [comp](const Level& l, Price p) { return comp(l.price, p); });

    if (it == levels.end() || it->price != price) return;

    if (it->quantity <= quantity) {
        levels.erase(it);
    } else {
        it->quantity -= quantity;
    }
}

// Public API
void OrderBook::add_order(const Order& order) {
    orders_.emplace(order.id, order);
    if (order.side == Side::Bid)
        add_level(bidLevels_, order.price, order.quantity, bidComp());
    else
        add_level(askLevels_, order.price, order.quantity, askComp());
}

bool OrderBook::cancel_order(OrderId id) {
    auto it = orders_.find(id);
    if (it == orders_.end()) return false;

    const Order& o = it->second;
    if (o.side == Side::Bid)
        remove_level(bidLevels_, o.price, o.quantity, bidComp());
    else
        remove_level(askLevels_, o.price, o.quantity, askComp());

    orders_.erase(it);
    return true;
}

bool OrderBook::modify_order(OrderId id, Quantity newQuantity) {
    auto it = orders_.find(id);
    if (it == orders_.end()) return false;

    Order& o = it->second;
    Quantity delta = newQuantity - o.quantity;

    if (o.side == Side::Bid)
        add_level(bidLevels_, o.price, delta, bidComp());
    else
        add_level(askLevels_, o.price, delta, askComp());

    o.quantity = newQuantity;
    return true;
}

bool OrderBook::execute_order(OrderId id, Quantity execQuantity) {
    auto it = orders_.find(id);
    if (it == orders_.end()) return false;

    Order& o = it->second;
    Quantity traded = std::min(execQuantity, o.quantity);

    if (o.side == Side::Bid)
        remove_level(bidLevels_, o.price, traded, bidComp());
    else
        remove_level(askLevels_, o.price, traded, askComp());

    o.quantity -= traded;
    if (o.quantity == 0) orders_.erase(it);

    return true;
}

std::pair<Price, Price> OrderBook::get_best_prices() const {
    Price bestBid = bidLevels_.empty() ? 0 : bidLevels_.back().price;
    Price bestAsk = askLevels_.empty() ? 0 : askLevels_.back().price;
    return {bestBid, bestAsk};
}

} // namespace trading
