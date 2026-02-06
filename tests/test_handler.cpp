#include "market_data_handler.hpp"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace trading;

int main() {
    // Create queue
    MarketDataHandler::Queue queue;
    MarketDataHandler handler(queue);

    // --- Helper to push and pop a message ---
    auto push_and_pop = [&](MessageType type, const void* payload, size_t payload_size) -> MarketMessage* {
        uint8_t buffer[1 + 64] = {}; // extra space
        buffer[0] = static_cast<uint8_t>(type);
        std::memcpy(buffer + 1, payload, payload_size);

        handler.push_raw_message(buffer, 1 + payload_size);
        MarketMessage* msg = queue.pop();
        assert(msg != nullptr); // must have message
        assert(msg->type == type);
        return msg;
    };

    // --- Test AddOrder ---
    AddOrderMsg add{1, Side::Bid, 1000, 10};
    auto* msg = push_and_pop(MessageType::AddOrder, &add, sizeof(add));
    assert(msg->add.orderId == add.orderId);
    assert(msg->add.side == add.side);
    assert(msg->add.price == add.price);
    assert(msg->add.qty == add.qty);
    handler.release_message(msg);

    // --- Test CancelOrder ---
    CancelOrderMsg cancel{1};
    msg = push_and_pop(MessageType::CancelOrder, &cancel, sizeof(cancel));
    assert(msg->cancel.orderId == cancel.orderId);
    handler.release_message(msg);

    // --- Test ModifyOrder ---
    ModifyOrderMsg modify{1, 50};
    msg = push_and_pop(MessageType::ModifyOrder, &modify, sizeof(modify));
    assert(msg->modify.orderId == modify.orderId);
    assert(msg->modify.newQty == modify.newQty);
    handler.release_message(msg);

    // --- Test Execute ---
    ExecuteMsg exec{1, 20, 1020};
    msg = push_and_pop(MessageType::Execute, &exec, sizeof(exec));
    assert(msg->execute.orderId == exec.orderId);
    assert(msg->execute.qty == exec.qty);
    assert(msg->execute.price == exec.price);
    handler.release_message(msg);

    // --- Test Trade ---
    TradeMsg trade{1, 2, 30, 1010};
    msg = push_and_pop(MessageType::Trade, &trade, sizeof(trade));
    assert(msg->trade.buyOrderId == trade.buyOrderId);
    assert(msg->trade.sellOrderId == trade.sellOrderId);
    assert(msg->trade.qty == trade.qty);
    assert(msg->trade.price == trade.price);
    handler.release_message(msg);

    // --- Test BBOUpdate ---
    BBOUpdateMsg bbo{1005, 1015, 50, 60};
    msg = push_and_pop(MessageType::BBOUpdate, &bbo, sizeof(bbo));
    assert(msg->bbo.bestBid == bbo.bestBid);
    assert(msg->bbo.bestAsk == bbo.bestAsk);
    assert(msg->bbo.bidSize == bbo.bidSize);
    assert(msg->bbo.askSize == bbo.askSize);
    handler.release_message(msg);

    std::cout << "All MarketDataHandler tests passed!\n";
    return 0;
}
