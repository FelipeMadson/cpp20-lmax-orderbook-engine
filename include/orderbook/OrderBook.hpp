#pragma once

#include "Types.hpp"
#include "Order.hpp"
#include "PriceLevel.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <optional>
#include <memory>

namespace orderbook {

struct LevelDepth {
    Price price;
    Quantity quantity;
    size_t orderCount;
};

struct BookDepth {
    std::vector<LevelDepth> bids;
    std::vector<LevelDepth> asks;
};

class OrderBook {
public:
    OrderBook() = default;
    ~OrderBook() = default;

    // Proíbe cópias para evitar overhead acidental de alocação de memória
    OrderBook(const OrderBook&) = delete;
    OrderBook& operator=(const OrderBook&) = delete;

    // Inserção e matching determinístico de ordens com prioridade Price-Time FIFO
    std::vector<ExecutionReport> addOrder(
        OrderId id,
        Side side,
        OrderType type,
        Price price,
        Quantity quantity
    );

    // Cancelamento determinístico de ordem existente em O(1)
    bool cancelOrder(OrderId id);

    [[nodiscard]] std::optional<Price> getBestBid() const noexcept;
    [[nodiscard]] std::optional<Price> getBestAsk() const noexcept;
    [[nodiscard]] std::optional<Price> getSpread() const noexcept;
    [[nodiscard]] size_t totalOrders() const noexcept { return ordersMap_.size(); }

    [[nodiscard]] BookDepth getDepth(size_t maxLevels = 5) const;

private:
    std::vector<ExecutionReport> matchLimitOrder(Order& incoming);
    std::vector<ExecutionReport> matchMarketOrder(Order& incoming);

    // Bids ordenados de forma decrescente (melhor compra no topo)
    std::map<Price, PriceLevel, std::greater<Price>> bids_;

    // Asks ordenados de forma crescente (melhor venda no topo)
    std::map<Price, PriceLevel, std::less<Price>> asks_;

    // Hash map contendo todas as ordens ativas para cancelamento O(1)
    std::unordered_map<OrderId, Order> ordersMap_;
};

} // namespace orderbook
