# ADR-002: Cache-Line Alignment and False Sharing Prevention

## Status
Accepted

## Context
Modern x86_64 and ARM64 CPUs fetch data into L1/L2 caches in fixed chunks of 64 bytes (cache lines). In multi-threaded lock-free architectures, when independent threads modify adjacent variables that reside within the same 64-byte cache line, the CPU's cache coherency protocol (MESI/MOESI) repeatedly invalidates the entire cache line across all CPU cores. This performance hazard, known as **false sharing**, degrades memory bus bandwidth and inflates latency by up to 20x.

## Decision
We enforce strict cache-line boundary isolation across all critical concurrency data structures:
1. **Explicit Alignment**: Sequence counters and circular buffer boundaries use C++20 `alignas(64)`:
   ```cpp
   alignas(64) std::atomic<uint64_t> cursor_{0};
   alignas(64) std::atomic<uint64_t> cached_gating_sequence_{0};
   ```
2. **Padding Arrays**: Explicit 56-byte dummy padding (`char pad[56]`) isolates atomic integers from neighbouring memory words.
3. **Cache Line Size Validation**: Compile-time static assertions verify that `sizeof(RingBufferSlot) % 64 == 0` or aligns to CPU L1 line boundaries.

## Consequences
- **Positive**: Eliminates cross-core cache invalidation storms during parallel order ingestion and execution event dispatching.
- **Trade-off**: Slight increase in memory footprint due to 64-byte stride padding, completely negligible for modern trading infrastructure memory budgets.
