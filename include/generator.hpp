#pragma once
#include "market_data_handler.hpp"
#include "config.hpp"
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <cstring>
#include <algorithm>

namespace trading {

class FeedGenerator {
public:
    FeedGenerator(MarketDataHandler& handler)
        : handler_(handler), stop_(false) {}

    void start() {
        thread_ = std::thread(&FeedGenerator::run, this);
    }

    void stop() {
        stop_ = true;
        if (thread_.joinable()) thread_.join();
    }

private:
    MarketDataHandler& handler_;
    std::thread thread_;
    bool stop_;
    std::vector<OrderId> activeOrders_; // track existing order IDs

    void run() {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::uniform_int_distribution<uint64_t> id_dist(1, 1000000);
        std::uniform_int_distribution<int> side_dist(0, 1); // 0=Buy,1=Sell
        std::uniform_int_distribution<uint32_t> qty_dist(1, 100);
        std::normal_distribution<double> price_dist(150, 50); // mid-market 150, sigma 50
        std::uniform_int_distribution<int> action_dist(0, 99); // 0-99 for probability

        while (!stop_) {
            int action = action_dist(gen);
            std::vector<uint8_t> buffer(1);

            if (action < 50) { // AddOrder
                AddOrderMsg msg{};
                msg.orderId = id_dist(gen);
                msg.side = static_cast<Side>(side_dist(gen));

                msg.price = clamp_price(static_cast<Price>(price_dist(gen)));
                msg.qty = qty_dist(gen);

                buffer.resize(1 + sizeof(AddOrderMsg));
                buffer[0] = static_cast<uint8_t>(MessageType::AddOrder);
                std::memcpy(buffer.data() + 1, &msg, sizeof(msg));

                activeOrders_.push_back(msg.orderId);

            } else if (action < 70 && !activeOrders_.empty()) { // CancelOrder
                CancelOrderMsg msg{};
                msg.orderId = activeOrders_[gen() % activeOrders_.size()];

                buffer.resize(1 + sizeof(CancelOrderMsg));
                buffer[0] = static_cast<uint8_t>(MessageType::CancelOrder);
                std::memcpy(buffer.data() + 1, &msg, sizeof(msg));

            } else if (action < 85 && !activeOrders_.empty()) { // ModifyOrder
                ModifyOrderMsg msg{};
                msg.orderId = activeOrders_[gen() % activeOrders_.size()];
                msg.newQty = qty_dist(gen);

                buffer.resize(1 + sizeof(ModifyOrderMsg));
                buffer[0] = static_cast<uint8_t>(MessageType::ModifyOrder);
                std::memcpy(buffer.data() + 1, &msg, sizeof(msg));

            } else if (!activeOrders_.empty()) { // Execute
                ExecuteMsg msg{};
                msg.orderId = activeOrders_[gen() % activeOrders_.size()];
                msg.qty = qty_dist(gen);

                msg.price = clamp_price(static_cast<Price>(price_dist(gen)));

                buffer.resize(1 + sizeof(ExecuteMsg));
                buffer[0] = static_cast<uint8_t>(MessageType::Execute);
                std::memcpy(buffer.data() + 1, &msg, sizeof(msg));
            }

            // Push message to handler
            handler_.push_raw_message(buffer.data(), buffer.size());

            // Simulate rate variability
            std::this_thread::sleep_for(std::chrono::microseconds(500 + (gen() % 1000)));
        }
    }
};

} // namespace trading
