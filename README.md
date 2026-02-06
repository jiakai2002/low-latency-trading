C++ Low-Latency Exchange Simulator

High-performance order book and market feed handler for ultra-low-latency trading experiments.

## Key Features

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
  - Mean : 0.26 µs
  - P50 :  0.21 µs
  - P90 :  0.25 µs
  - P99 :  0.33 µs
  - P99.9 : 3.25 µs
- **Best Bid/Ask Lookup**: 
  - Mean : 0.3 µs
- **Cancel Order**: 
  - Mean : 0.19 µs
  - P50 : 0.17 µs
  - P90 : 0.21 µs
  - P99 : 0.25 µs
  - P99.9: 1.29 µs

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
