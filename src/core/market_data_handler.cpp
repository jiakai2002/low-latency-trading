#include "../../include/core/market_data_handler.hpp"

namespace trading {

MarketDataHandler::MarketDataHandler(Queue& queue)
    : queue_(queue) {}

/*/
+-------------------+---------------------------+
| 1 byte            | N bytes                   |
+-------------------+---------------------------+
| MessageType value | Message payload (union)   |
+-------------------+---------------------------+
/*/
void MarketDataHandler::push_raw_message(const uint8_t* buffer, size_t size) {
    if (size < 1) return;

    // copy type
    auto type = static_cast<MessageType>(buffer[0]);
    auto msg = pool_.allocate();
    if (!msg) return;
    msg->type = type;

    // copy payload
    switch (type) {
        case MessageType::AddOrder:
            if (size >= sizeof(AddOrderMsg) + 1)
                std::memcpy(&msg->add, buffer + 1, sizeof(AddOrderMsg));
            break;

        case MessageType::CancelOrder:
            if (size >= sizeof(CancelOrderMsg) + 1)
                std::memcpy(&msg->cancel, buffer + 1, sizeof(CancelOrderMsg));
            break;

        case MessageType::ModifyOrder:
            if (size >= sizeof(ModifyOrderMsg) + 1)
                std::memcpy(&msg->modify, buffer + 1, sizeof(ModifyOrderMsg));
            break;

        case MessageType::Execute:
            if (size >= sizeof(ExecuteMsg) + 1)
                std::memcpy(&msg->execute, buffer + 1, sizeof(ExecuteMsg));
            break;

        case MessageType::Trade:
            if (size >= sizeof(TradeMsg) + 1)
                std::memcpy(&msg->trade, buffer + 1, sizeof(TradeMsg));
            break;

        case MessageType::BBOUpdate:
            if (size >= sizeof(BBOUpdateMsg) + 1)
                std::memcpy(&msg->bbo, buffer + 1, sizeof(BBOUpdateMsg));
            break;

        default:
            break;
    }

    // push into queue
    if (!queue_.push(msg)) {
        pool_.release(msg);
        return;
    }

}

} // namespace trading