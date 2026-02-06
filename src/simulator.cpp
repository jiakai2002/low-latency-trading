#include "order_book.hpp"
#include "market_data_handler.hpp"
#include "generator.hpp"
#include "lock_free_queue.hpp"
#include "logger.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using namespace trading;

int main() {
    Logger logger(5000);

    MarketDataHandler::Queue queue;
    MarketDataHandler handler(queue);

    OrderBook book;

    std::atomic<bool> stop{false};
    std::atomic<uint64_t> ordersProcessed{0};
    std::atomic<uint64_t> ordersExecuted{0};
    std::atomic<uint64_t> ordersCancelled{0};

    // Consumer thread: pop messages and apply to order book
    std::thread consumer([&]() {
        while (!stop.load()) {
            MarketMessage* msg = queue.pop();
            if (!msg) {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
                continue;
            }

            ++ordersProcessed;

            switch (msg->type) {
                case MessageType::AddOrder: {
                    Order o;
                    o.id = msg->add.orderId;
                    o.side = msg->add.side;
                    o.price = msg->add.price;
                    o.quantity = msg->add.qty;
                    book.add_order(o);
                    logger.log("AddOrder: id=" + std::to_string(o.id) +
                            ", side=" + (o.side == Side::Bid ? "Bid" : "Ask") +
                            ", price=" + std::to_string(o.price) +
                            ", qty=" + std::to_string(o.quantity));
                    break;
                }
                case MessageType::CancelOrder:
                    book.cancel_order(msg->cancel.orderId);
                    logger.log("CancelOrder: id=" + std::to_string(msg->cancel.orderId));
                    ordersCancelled++;
                    break;
                case MessageType::ModifyOrder:
                    book.modify_order(msg->modify.orderId, msg->modify.newQty);
                    logger.log("ModifyOrder: id=" + std::to_string(msg->modify.orderId) +
                            ", newQty=" + std::to_string(msg->modify.newQty));
                    break;
                case MessageType::Execute:
                    book.execute_order(msg->execute.orderId, msg->execute.qty);
                    ordersExecuted++;
                    logger.log("ExecuteOrder: id=" + std::to_string(msg->execute.orderId) +
                            ", qty=" + std::to_string(msg->execute.qty));
                    break;
                case MessageType::Trade:
                case MessageType::BBOUpdate:
                    break;
            }

            // Print stats immediately after processing this message
            auto bestBid = book.get_best_bid().value_or(0);
            auto bestAsk = book.get_best_ask().value_or(0);

            size_t queueSize = queue.approx_size();

            logger.log(
                "[Stats] BestBid: " + std::to_string(bestBid) +
                ", BestAsk: " + std::to_string(bestAsk) +
                ", Processed: " + std::to_string(ordersProcessed.load()) +
                "\n"
            );

            if (ordersProcessed % 50 == 0) {
                logger.print();
            }

            handler.release_message(msg);
        }

        // Drain remaining messages
        while (MarketMessage* msg = queue.pop()) {
            handler.release_message(msg);
        }
    });

    // Start feed generator
    FeedGenerator generator(handler);
    generator.start();

    std::cout << "Simulation running. Press Ctrl+C to stop.\n";

    // Keep main thread alive until user interrupts
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Cleanup (will not reach unless you implement a signal handler)
    stop.store(true);
    generator.stop();
    consumer.join();

    return 0;
}
