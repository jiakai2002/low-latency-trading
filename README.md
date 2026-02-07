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

#### Order Book Operations
- **Add Order**: 
  - Mean : 35 ns
  - P50 :  41 ns
  - P90 :  42 ns
  - P99 :  42 ns
  - P99.9 : 2375 ns
- **Best Bid/Ask Lookup**: 
  - Mean : 16 ns
  - P99.9 : 42 ns
- **Cancel Order**: 
  - Mean : 31 ns
  - P50 : 41 ns
  - P90 : 42 ns
  - P99 : 42 ns
  - P99.9: 1417 ns

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
