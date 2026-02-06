#include "order_book.hpp"
#include "config.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>

using namespace trading;

void test_order_book() {
    OrderBook book;

    // Add bids and asks
    book.add_order({1, Side::Bid, clamp_price(50), 10});
    book.add_order({2, Side::Bid, clamp_price(55), 5});
    book.add_order({3, Side::Ask, clamp_price(60), 8});
    book.add_order({4, Side::Ask, clamp_price(58), 12});

    // Check best prices
    {
        auto bestBid = book.get_best_bid();
        auto bestAsk = book.get_best_ask();
        assert(bestBid.has_value());
        assert(bestAsk.has_value());
        assert(*bestBid == 55); // highest bid
        assert(*bestAsk == 58); // lowest ask
    }

    // Modify an order
    bool modified = book.modify_order(1, 15);
    assert(modified);

    // Execute part of a bid
    bool executed = book.execute_order(2, 3);
    assert(executed);

    // Execute remaining quantity of a bid (should remove it)
    executed = book.execute_order(2, 2);
    assert(executed);

    // Cancel an ask
    bool canceled = book.cancel_order(3);
    assert(canceled);

    // Check remaining best bid/ask
    auto bestBid = book.get_best_bid().value_or(0);
    auto bestAsk = book.get_best_ask().value_or(0);

    // Remaining bids: order 1 (50, 15)
    // Remaining asks: order 4 (58, 12)
    assert(bestBid == 50);
    assert(bestAsk == 58);

    std::cout << "All OrderBook tests passed!\n";
}

int main() {
    test_order_book();
    return 0;
}
