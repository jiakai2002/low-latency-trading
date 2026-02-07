## C++ Low-Latency Exchange Simulator

High-performance order book and market feed handler for ultra-low-latency trading.

## Optimizations

- Sub-microsecond order book operations
- Lock-free queues for concurrency
- Zero-copying message handling
- Cache-line aligned data structures
- Memory pool allocation
- Bid price normalization (store as negative) – avoids branch mispredictions in the hot path for fast best-bid/best-ask calculations.

## Performance Highlights

### Benchmark Results (M1 iMac, macOS Sonoma)
 
<img width="421" height="747" alt="Screenshot 2026-02-07 at 4 25 51 PM" src="https://github.com/user-attachments/assets/cad7e540-ba09-4c55-8aca-f33dab054891" />

## Getting Started

### Build Instructions
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Run Benchmark
```bash
# From build directory
./benchmark
```

### Run Market Simulator
```bash
# From build directory
./simulator
```

## Contributions
If you find potential for optimization, feel free to feedback!
