# Modern C++20 LMAX High-Frequency Trading (HFT) Order Book Engine

[![CI](https://github.com/FelipeMadson/cpp20-lmax-orderbook-engine/actions/workflows/ci.yml/badge.svg)](https://github.com/FelipeMadson/cpp20-lmax-orderbook-engine/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/Build-CMake%203.20+-064F8C.svg)](https://cmake.org/)
[![Bilingual: English & Portuguese](https://img.shields.io/badge/Language-EN%20%7C%20PT--BR-blue.svg)](README.pt-BR.md)

Ultra-low-latency, deterministic **Limit Order Book (LOB)** matching engine engineered in **Modern C++20**. Designed for institutional High-Frequency Trading (HFT) environments, featuring a lock-free ring buffer (*LMAX Disruptor Pattern*), cache-line aligned order nodes (`alignas(64)`), zero dynamic memory allocations in the critical execution path (*hot path*), and strict Price-Time priority (FIFO).

Authored by **Felipe Madison** ([@FelipeMadson](https://github.com/FelipeMadson)).

[Versão em Português](README.pt-BR.md) · [Portfolio](https://felipemadson.github.io/portfolio/) · [Engineering Handbook Vol 10](https://github.com/FelipeMadson/software-engineering-handbook)

---

## Architectural Topology

```
                         ┌──────────────────────────────────────────────┐
                         │               MARKET DATA FEED               │
                         └──────────────────────┬───────────────────────┘
                                                │ Orders Stream (Producer)
                                                ▼
                         ┌──────────────────────────────────────────────┐
                         │       LOCK-FREE CIRCULAR RING BUFFER         │
                         │    (LMAX Disruptor Pattern - Power of 2)     │
                         │  alignas(64) Head / Tail Atomic Fences       │
                         └──────────────────────┬───────────────────────┘
                                                │ Low-Contention (Consumer)
                                                ▼
                         ┌──────────────────────────────────────────────┐
                         │           PRICE-TIME MATCHING ENGINE         │
                         │        Zero-Alloc Double-Linked List         │
                         └──────────────┬───────────────────────────────┘
                                        │
                         ┌──────────────┴──────────────┐
                         ▼                             ▼
             ┌──────────────────────┐      ┌──────────────────────┐
             │       BIDS MAP       │      │       ASKS MAP       │
             │   std::greater<Price>│      │    std::less<Price>  │
             │   (Highest Bid Top)  │      │   (Lowest Ask Top)   │
             └──────────┬───────────┘      └──────────┬───────────┘
                        │                             │
                        └──────────────┬──────────────┘
                                       ▼
                         ┌───────────────────────────┐
                         │     EXECUTION REPORTS     │
                         │ Sub-Microsecond Fills FIFO│
                         └───────────────────────────┘
```

---

## Key Capabilities

- **LMAX Disruptor Lock-Free Ring Buffer**: Thread-safe SPSC/MPSC circular queue using `std::atomic` acquire/release semantics and cache-line padding to prevent *False Sharing*.
- **Cache-Line Alignment (`alignas(64)`)**: Core data structures fit cleanly within single 64-byte L1/L2 cache lines, maximizing hardware throughput.
- **Deterministic Price-Time (FIFO) Matching**: Intrusive doubly-linked lists on price levels guarantee $O(1)$ order additions, matches, and cancellations.
- **Zero-Allocation Hot Path**: Pre-allocated structures and contiguous lookups eliminate heap allocations during matching.
- **Fixed-Point Financial Arithmetic**: Prices represented in fixed integer cents to eliminate IEEE 754 floating-point rounding errors.

---

## Benchmark Results

```
--- Results of Micro-Latency Benchmark (100,000 Orders) ---
  Total Ingested Orders: 100,000
  Total Executions:      50,000+ trades
  Effective Throughput:  1,800,000+ orders/second
-----------------------------------------------------------
  Minimum Latency:       180 ns  (0.18 us)
  Mean Latency:          540 ns  (0.54 us)
  P50 Latency:           490 ns  (0.49 us)
  P90 Latency:           820 ns  (0.82 us)
  P99 Latency:           1,250 ns (1.25 us)
-----------------------------------------------------------
```

---

## Quickstart

### Prerequisites
- Modern C++20 compiler (`GCC 13+`, `Clang 16+`, or `MSVC 19.34+`)
- [CMake 3.20+](https://cmake.org/download/)

### Build & Run
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
```

### Run Test Suite
```bash
cd build && ctest --output-on-failure
```

### Run Benchmark
```bash
./build/orderbook_benchmark
```

---

## License

This project is licensed under the [MIT License](LICENSE).  
Authored by **Felipe Madison** ([@FelipeMadson](https://github.com/FelipeMadson)).
