#pragma once
#include "memory_pool.hpp"
#include "lock_free_queue.hpp"
#include "order_book.hpp"
#include <cstdint>
#include <functional>
#include <cstring>

#pragma pack(push, 1)

namespace trading {

enum class MessageType : uint8_t {
    AddOrder,
    CancelOrder,
    ModifyOrder,
    Execute,
    Trade,
    BBOUpdate
};

struct AddOrderMsg { OrderId orderId; Side side; Price price; Quantity qty; };
struct CancelOrderMsg { OrderId orderId; };
struct ModifyOrderMsg { OrderId orderId; Quantity newQty; };
struct ExecuteMsg { OrderId orderId; Quantity qty; Price price; };
struct TradeMsg { uint64_t buyOrderId; uint64_t sellOrderId; Quantity qty; Price price; };
struct BBOUpdateMsg { Price bestBid; Price bestAsk; Quantity bidSize; Quantity askSize; };

struct MarketMessage {
    MessageType type;
    union {
        AddOrderMsg add;
        CancelOrderMsg cancel;
        ModifyOrderMsg modify;
        ExecuteMsg execute;
        TradeMsg trade;
        BBOUpdateMsg bbo;
    };
};

#pragma pack(pop)

class MarketDataHandler {
public:
    using Queue = LockFreeQueue<MarketMessage, 4096>;

    explicit MarketDataHandler(Queue& queue);

    void push_raw_message(const uint8_t* buffer, size_t size);

    void release_message(MarketMessage* msg) {pool_.release(msg);}

private:
    Queue& queue_;
    MemoryPool<MarketMessage, 4096> pool_;
};

}