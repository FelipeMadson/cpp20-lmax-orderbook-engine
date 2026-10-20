#pragma once

#include "Order.hpp"
#include <cstddef>

namespace orderbook {

// Nível de Preço com fila intrusiva duplamente encadeada (O(1) add, remove, match)
class PriceLevel {
public:
    explicit PriceLevel(Price price) noexcept
        : price_(price), totalQuantity_(0), orderCount_(0), head_(nullptr), tail_(nullptr) {}

    void append(Order* order) noexcept {
        order->prev = tail_;
        order->next = nullptr;

        if (tail_) {
            tail_->next = order;
        } else {
            head_ = order;
        }
        tail_ = order;

        totalQuantity_ += order->remainingQuantity;
        orderCount_++;
    }

    void remove(Order* order) noexcept {
        if (order->prev) {
            order->prev->next = order->next;
        } else {
            head_ = order->next;
        }

        if (order->next) {
            order->next->prev = order->prev;
        } else {
            tail_ = order->prev;
        }

        totalQuantity_ -= order->remainingQuantity;
        orderCount_--;

        order->prev = nullptr;
        order->next = nullptr;
    }

    [[nodiscard]] Price price() const noexcept { return price_; }
    [[nodiscard]] Quantity totalQuantity() const noexcept { return totalQuantity_; }
    [[nodiscard]] size_t orderCount() const noexcept { return orderCount_; }
    [[nodiscard]] bool empty() const noexcept { return head_ == nullptr; }
    [[nodiscard]] Order* head() noexcept { return head_; }

    void reduceQuantity(Quantity delta) noexcept {
        if (delta >= totalQuantity_) {
            totalQuantity_ = 0;
        } else {
            totalQuantity_ -= delta;
        }
    }

private:
    Price price_;
    Quantity totalQuantity_;
    size_t orderCount_;
    Order* head_;
    Order* tail_;
};

} // namespace orderbook
