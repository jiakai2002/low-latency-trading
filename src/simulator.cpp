#include "order_book.hpp"
#include "market_data_handler.hpp"
#include "generator.hpp"
#include "lock_free_queue.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using namespace trading;

int main() {
    MarketDataHandler::Queue queue;
    MarketDataHandler handler(queue);

    OrderBook book;

    std::atomic<bool> stop{false};

    // Consumer thread: pop messages from queue and apply to order book
    std::thread consumer([&]() {
        while (!stop.load()) {
            MarketMessage* msg = queue.pop();
            if (!msg) {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
                continue;
            }

            switch (msg->type) {
                case MessageType::AddOrder: {
                    Order o;
                    o.id = msg->add.orderId;
                    o.side = msg->add.side;
                    o.price = msg->add.price;
                    o.quantity = msg->add.qty;
                    book.add_order(o);
                    break;
                }
                case MessageType::CancelOrder:
                    book.cancel_order(msg->cancel.orderId);
                    break;
                case MessageType::ModifyOrder:
                    book.modify_order(msg->modify.orderId, msg->modify.newQty);
                    break;
                case MessageType::Execute:
                    book.execute_order(msg->execute.orderId, msg->execute.qty);
                    break;
                case MessageType::Trade:
                    break;
                case MessageType::BBOUpdate:
                    break;
            }

            handler.release_message(msg);
        }

        // Drain remaining messages
        while (MarketMessage* msg = queue.pop()) {
            handler.release_message(msg);
        }
    });

    // FeedGenerator
    FeedGenerator generator(handler);
    generator.start();

    // Run simulation for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Stop generator and consumer
    generator.stop();
    stop.store(true);
    consumer.join();

    auto bestBid = book.get_best_bid().value_or(0);
    auto bestAsk = book.get_best_ask().value_or(0);
    std::cout << "Final Best Bid: " << bestBid << ", Best Ask: " << bestAsk << "\n";

    return 0;
}
