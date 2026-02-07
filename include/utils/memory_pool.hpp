#pragma once
#include <array>
#include <cstddef>

template <typename T, size_t N>
class MemoryPool {
public:
    MemoryPool() : freeCount(N) {
        for (size_t i = 0; i < N; ++i) freeSlots_[i] = &storage[i];
    }

    T* allocate() {
        if (freeCount == 0) return nullptr;
        return freeSlots_[--freeCount];
    }

    void release(T* ptr) {
        freeSlots_[freeCount++] = ptr;
    }

private:
    std::array<T, N> storage;
    std::array<T*, N> freeSlots_;
    size_t freeCount;
};
