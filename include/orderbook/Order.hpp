#pragma once

#include "Types.hpp"
#include <new>

namespace orderbook {

// Estrutura alinhada em linha de cache (64 bytes) para eliminar False Sharing
struct alignas(64) Order {
    OrderId id{0};
    Price price{0};
    Quantity initialQuantity{0};
    Quantity remainingQuantity{0};
    Side side{Side::Buy};
    OrderType type{OrderType::Limit};
    Timestamp timestampNs{0};
    
    // Ponteiros para lista encadeada intrusiva no nível de preço (PriceLevel)
    Order* prev{nullptr};
    Order* next{nullptr};

    [[nodiscard]] constexpr bool isFilled() const noexcept {
        return remainingQuantity == 0;
    }

    constexpr void fill(Quantity qty) noexcept {
        if (qty >= remainingQuantity) {
            remainingQuantity = 0;
        } else {
            remainingQuantity -= qty;
        }
    }
};

} // namespace orderbook
