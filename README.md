Low-Latency Trading Systems Framework

## Overview

This is my approach for a high-performance, production-grade C++ framework for building ultra-low-latency trading systems, used for quantitative finance and systems programming. My focus was on highly efficient and especially clean code.
I saw this project as a challenge, trying to push as hard as I can, seeing what I can get done if I give it my all.

## Key Features

- 🚀 Sub-microsecond latency processing (blazingly fast IMO)
- 🔬 Statistical arbitrage strategy
- 🧊 Lock-free concurrent data structures
- 📊 Market data simulation (though, only a simulation)
- 🔍 Detailed performance benchmarking (run them yourself =)!)

## Performance Highlights

### Benchmark Results (M1 iMac, Sonoma)

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

#### Lock-Free Queue
- **Push/Pop Operations**: 
  - Mean Latency: ~14.5 ns
  - Consistently under 42 ns at 99th percentile

## Technical Architecture

### Components
- **Order Book**: Optimized 
- **Market Data Handler**: Real-time binary market data feed processing

### Optimizations
- Zero-overhead abstractions
- Cache-line optimized data structures
- Statistical arbitrage modeling

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
