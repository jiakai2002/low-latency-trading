#pragma once
#include <array>
#include <atomic>
#include <cstddef>

template <typename T, size_t N>
class LockFreeQueue {
public:
    LockFreeQueue() : head_(0), tail_(0) {}

    bool push(T* item) {
        size_t next = (head_ + 1) % N;
        // acquire work done by other threads
        if (next == tail_.load(std::memory_order_acquire)) return false; // full
        buffer_[head_] = item;
        head_ = next;
        return true;
    }

    T* pop() {
        // producer reads via atomic so must be atomic
        size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_) return nullptr; // empty
        T* item = buffer_[tail];
        // release work done to other threads
        tail_.store((tail + 1) % N, std::memory_order_release);
        return item;
    }

private:
    T* buffer_[N];
    size_t head_;   // producer only
    // tail is written by consumer, read by producer
    // separate cache line to avoid false sharing
    alignas(64) std::atomic<size_t> tail_; // consumer only
};
