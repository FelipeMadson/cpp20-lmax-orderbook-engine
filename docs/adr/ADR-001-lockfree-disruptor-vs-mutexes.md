# ADR-001: Lock-Free Single-Writer Disruptor vs Mutexes

## Status
Accepted

## Context
In ultra-low latency High-Frequency Trading (HFT) limit order matching systems, traditional OS-level synchronization primitives (such as `std::mutex` or `pthread_mutex`) introduce non-deterministic execution times. When multiple threads contend for locks, kernel transitions (futexes), context switching, and CPU cache thrashing lead to tail latencies exceeding several microseconds, causing unacceptable slippage and order queue penalties.

## Decision
We adopted the **LMAX Disruptor pattern** utilizing a single-writer lock-free ring buffer with atomic sequence numbers:
1. **Single Writer Principle**: Only the matching thread mutates order book depth states, eliminating data races on internal book levels without mutexes.
2. **Atomic Head/Tail Pointers**: Sequence advancement relies on `std::atomic<uint64_t>` with `std::memory_order_release` and `std::memory_order_acquire`.
3. **Bounded Circular Buffer**: The ring buffer size is constrained to powers of two (`2^N`), enabling index wrapping via bitwise AND (`seq & mask`) instead of expensive modulo division.

## Consequences
- **Positive**: Sub-microsecond deterministic P99 execution latency (< 350ns); zero kernel syscalls during normal order processing.
- **Trade-off**: Requires dedicated thread pinning (core isolation) and pre-allocated memory pools to avoid OS scheduler interruptions.
