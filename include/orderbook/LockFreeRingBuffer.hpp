#pragma once

#include <atomic>
#include <cstddef>
#include <vector>
#include <optional>
#include <new>

namespace orderbook {

// Buffer circular Lock-Free com semântica de aquisição/liberação (LMAX Disruptor Pattern)
// Capacidade deve ser potência de 2 para otimização com bitwise AND (masking)
template <typename T, size_t Capacity = 65536>
class LockFreeRingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "A capacidade do RingBuffer deve ser uma potência de 2.");

public:
    LockFreeRingBuffer() : buffer_(Capacity) {
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    // Tenta enfileirar um item. Retorna false se o buffer estiver saturado.
    bool push(const T& item) noexcept {
        const size_t currentTail = tail_.load(std::memory_order_relaxed);
        const size_t currentHead = head_.load(std::memory_order_acquire);

        if ((currentTail - currentHead) >= Capacity) {
            return false; // Buffer cheio (backpressure)
        }

        buffer_[currentTail & Mask] = item;
        tail_.store(currentTail + 1, std::memory_order_release);
        return true;
    }

    // Tenta desenfileirar um item. Retorna false se o buffer estiver vazio.
    bool pop(T& item) noexcept {
        const size_t currentHead = head_.load(std::memory_order_relaxed);
        const size_t currentTail = tail_.load(std::memory_order_acquire);

        if (currentHead >= currentTail) {
            return false; // Buffer vazio
        }

        item = buffer_[currentHead & Mask];
        head_.store(currentHead + 1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] size_t size() const noexcept {
        const size_t currentHead = head_.load(std::memory_order_acquire);
        const size_t currentTail = tail_.load(std::memory_order_acquire);
        return (currentTail >= currentHead) ? (currentTail - currentHead) : 0;
    }

    [[nodiscard]] bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept {
        return Capacity;
    }

private:
    static constexpr size_t Mask = Capacity - 1;

    // Cache-line padding (64 bytes) para evitar false sharing entre produtores e consumidores
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};
    std::vector<T> buffer_;
};

} // namespace orderbook
