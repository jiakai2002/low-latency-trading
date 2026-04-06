#pragma once
#include <array>
#include <atomic>
#include <cstddef>

template <typename T, size_t N>
class LockFreeQueue {
public:
    LockFreeQueue() : head_(0), tail_(0) {}

    // Producer: push
    bool push(T* item) {
        size_t next = (head_ + 1) % N;

        // Acquire ensures we see latest tail updated by consumer
        if (next == tail_.load(std::memory_order_acquire)) return false; // full

        // Write the item
        buffer_[head_] = item;

        // Release fence ensures item write is visible before updating head_
        std::atomic_thread_fence(std::memory_order_release);

        // Update head (plain is okay, producer only)
        head_ = next;

        return true;
    }

    // Consumer: pop
    T* pop() {
        // Read current tail
        size_t current_tail = tail_.load(std::memory_order_relaxed);

        // Acquire fence ensures we see producer's write to buffer before reading
        std::atomic_thread_fence(std::memory_order_acquire);

        // Check if empty
        if (current_tail == head_) return nullptr;

        // Read the item
        T* item = buffer_[current_tail];

        // Update tail (release ensures consumer progress visible to producer)
        tail_.store((current_tail + 1) % N, std::memory_order_release);

        return item;
    }

private:
    T* buffer_[N];
    size_t head_;   // producer only
    // tail is written by consumer, read by producer
    // separate cache line to avoid false sharing
    alignas(64) std::atomic<size_t> tail_; // consumer only
};