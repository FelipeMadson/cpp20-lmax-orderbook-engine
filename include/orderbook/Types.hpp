#pragma once

#include <cstdint>
#include <string_view>
#include <chrono>

namespace orderbook {

using OrderId = uint64_t;
using Price = uint64_t;       // Preço representado em centavos/pontos fixos (fixed-point arithmetic)
using Quantity = uint32_t;    // Quantidade de ativos/ações
using Timestamp = uint64_t;   // Timestamp em nanossegundos desde epoch

enum class Side : uint8_t {
    Buy = 0,
    Sell = 1
};

enum class OrderType : uint8_t {
    Limit = 0,
    Market = 1
};

enum class MatchStatus : uint8_t {
    Placed = 0,
    PartiallyFilled = 1,
    Filled = 2,
    Cancelled = 3,
    Rejected = 4
};

// Estrutura de execução/trade gerada pelo motor de matching
struct ExecutionReport {
    OrderId buyOrderId;
    OrderId sellOrderId;
    Price matchPrice;
    Quantity matchQuantity;
    Timestamp executionTimeNs;
};

inline Timestamp getCurrentTimestampNs() noexcept {
    return static_cast<Timestamp>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count()
    );
}

inline std::string_view sideToString(Side side) noexcept {
    return side == Side::Buy ? "BUY" : "SELL";
}

} // namespace orderbook
