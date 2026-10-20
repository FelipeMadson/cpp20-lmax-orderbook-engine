#include "orderbook/OrderBook.hpp"
#include <algorithm>

namespace orderbook {

std::vector<ExecutionReport> OrderBook::addOrder(
    OrderId id,
    Side side,
    OrderType type,
    Price price,
    Quantity quantity
) {
    std::vector<ExecutionReport> reports;
    if (quantity == 0 || (type == OrderType::Limit && price == 0)) {
        return reports;
    }

    if (ordersMap_.find(id) != ordersMap_.end()) {
        return reports; // Rejeita duplicação de ID de ordem
    }

    Order newOrder{
        .id = id,
        .price = price,
        .initialQuantity = quantity,
        .remainingQuantity = quantity,
        .side = side,
        .type = type,
        .timestampNs = getCurrentTimestampNs(),
        .prev = nullptr,
        .next = nullptr
    };

    if (type == OrderType::Market) {
        return matchMarketOrder(newOrder);
    }

    reports = matchLimitOrder(newOrder);

    // Se ainda restou quantidade e a ordem era Limit, insere no book (Resting Order)
    if (newOrder.remainingQuantity > 0) {
        auto [it, inserted] = ordersMap_.emplace(id, newOrder);
        Order* orderPtr = &it->second;

        if (side == Side::Buy) {
            auto levelIt = bids_.find(price);
            if (levelIt == bids_.end()) {
                levelIt = bids_.emplace(price, PriceLevel(price)).first;
            }
            levelIt->second.append(orderPtr);
        } else {
            auto levelIt = asks_.find(price);
            if (levelIt == asks_.end()) {
                levelIt = asks_.emplace(price, PriceLevel(price)).first;
            }
            levelIt->second.append(orderPtr);
        }
    }

    return reports;
}

std::vector<ExecutionReport> OrderBook::matchLimitOrder(Order& incoming) {
    std::vector<ExecutionReport> reports;

    if (incoming.side == Side::Buy) {
        // Casa contra o lado Ask (melhor preço de venda primeiro)
        auto askIt = asks_.begin();
        while (askIt != asks_.end() && incoming.remainingQuantity > 0 && incoming.price >= askIt->first) {
            PriceLevel& level = askIt->second;
            Order* resting = level.head();

            while (resting != nullptr && incoming.remainingQuantity > 0) {
                Order* nextResting = resting->next;
                Quantity tradeQty = std::min(incoming.remainingQuantity, resting->remainingQuantity);

                incoming.fill(tradeQty);
                resting->fill(tradeQty);
                level.reduceQuantity(tradeQty);

                reports.push_back(ExecutionReport{
                    .buyOrderId = incoming.id,
                    .sellOrderId = resting->id,
                    .matchPrice = resting->price,
                    .matchQuantity = tradeQty,
                    .executionTimeNs = getCurrentTimestampNs()
                });

                if (resting->isFilled()) {
                    OrderId restingId = resting->id;
                    level.remove(resting);
                    ordersMap_.erase(restingId);
                }

                resting = nextResting;
            }

            if (level.empty()) {
                askIt = asks_.erase(askIt);
            } else {
                ++askIt;
            }
        }
    } else {
        // Casa contra o lado Bid (melhor preço de compra primeiro)
        auto bidIt = bids_.begin();
        while (bidIt != bids_.end() && incoming.remainingQuantity > 0 && incoming.price <= bidIt->first) {
            PriceLevel& level = bidIt->second;
            Order* resting = level.head();

            while (resting != nullptr && incoming.remainingQuantity > 0) {
                Order* nextResting = resting->next;
                Quantity tradeQty = std::min(incoming.remainingQuantity, resting->remainingQuantity);

                incoming.fill(tradeQty);
                resting->fill(tradeQty);
                level.reduceQuantity(tradeQty);

                reports.push_back(ExecutionReport{
                    .buyOrderId = resting->id,
                    .sellOrderId = incoming.id,
                    .matchPrice = resting->price,
                    .matchQuantity = tradeQty,
                    .executionTimeNs = getCurrentTimestampNs()
                });

                if (resting->isFilled()) {
                    OrderId restingId = resting->id;
                    level.remove(resting);
                    ordersMap_.erase(restingId);
                }

                resting = nextResting;
            }

            if (level.empty()) {
                bidIt = bids_.erase(bidIt);
            } else {
                ++bidIt;
            }
        }
    }

    return reports;
}

std::vector<ExecutionReport> OrderBook::matchMarketOrder(Order& incoming) {
    std::vector<ExecutionReport> reports;

    if (incoming.side == Side::Buy) {
        auto askIt = asks_.begin();
        while (askIt != asks_.end() && incoming.remainingQuantity > 0) {
            PriceLevel& level = askIt->second;
            Order* resting = level.head();

            while (resting != nullptr && incoming.remainingQuantity > 0) {
                Order* nextResting = resting->next;
                Quantity tradeQty = std::min(incoming.remainingQuantity, resting->remainingQuantity);

                incoming.fill(tradeQty);
                resting->fill(tradeQty);
                level.reduceQuantity(tradeQty);

                reports.push_back(ExecutionReport{
                    .buyOrderId = incoming.id,
                    .sellOrderId = resting->id,
                    .matchPrice = resting->price,
                    .matchQuantity = tradeQty,
                    .executionTimeNs = getCurrentTimestampNs()
                });

                if (resting->isFilled()) {
                    OrderId restingId = resting->id;
                    level.remove(resting);
                    ordersMap_.erase(restingId);
                }

                resting = nextResting;
            }

            if (level.empty()) {
                askIt = asks_.erase(askIt);
            } else {
                ++askIt;
            }
        }
    } else {
        auto bidIt = bids_.begin();
        while (bidIt != bids_.end() && incoming.remainingQuantity > 0) {
            PriceLevel& level = bidIt->second;
            Order* resting = level.head();

            while (resting != nullptr && incoming.remainingQuantity > 0) {
                Order* nextResting = resting->next;
                Quantity tradeQty = std::min(incoming.remainingQuantity, resting->remainingQuantity);

                incoming.fill(tradeQty);
                resting->fill(tradeQty);
                level.reduceQuantity(tradeQty);

                reports.push_back(ExecutionReport{
                    .buyOrderId = resting->id,
                    .sellOrderId = incoming.id,
                    .matchPrice = resting->price,
                    .matchQuantity = tradeQty,
                    .executionTimeNs = getCurrentTimestampNs()
                });

                if (resting->isFilled()) {
                    OrderId restingId = resting->id;
                    level.remove(resting);
                    ordersMap_.erase(restingId);
                }

                resting = nextResting;
            }

            if (level.empty()) {
                bidIt = bids_.erase(bidIt);
            } else {
                ++bidIt;
            }
        }
    }

    return reports;
}

bool OrderBook::cancelOrder(OrderId id) {
    auto it = ordersMap_.find(id);
    if (it == ordersMap_.end()) {
        return false;
    }

    Order& order = it->second;
    if (order.side == Side::Buy) {
        auto levelIt = bids_.find(order.price);
        if (levelIt != bids_.end()) {
            levelIt->second.remove(&order);
            if (levelIt->second.empty()) {
                bids_.erase(levelIt);
            }
        }
    } else {
        auto levelIt = asks_.find(order.price);
        if (levelIt != asks_.end()) {
            levelIt->second.remove(&order);
            if (levelIt->second.empty()) {
                asks_.erase(levelIt);
            }
        }
    }

    ordersMap_.erase(it);
    return true;
}

std::optional<Price> OrderBook::getBestBid() const noexcept {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::getBestAsk() const noexcept {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

std::optional<Price> OrderBook::getSpread() const noexcept {
    auto bestBid = getBestBid();
    auto bestAsk = getBestAsk();
    if (bestBid && bestAsk && *bestAsk >= *bestBid) {
        return *bestAsk - *bestBid;
    }
    return std::nullopt;
}

BookDepth OrderBook::getDepth(size_t maxLevels) const {
    BookDepth depth;
    size_t count = 0;
    for (const auto& [price, level] : bids_) {
        if (count++ >= maxLevels) break;
        depth.bids.push_back(LevelDepth{
            .price = price,
            .quantity = level.totalQuantity(),
            .orderCount = level.orderCount()
        });
    }

    count = 0;
    for (const auto& [price, level] : asks_) {
        if (count++ >= maxLevels) break;
        depth.asks.push_back(LevelDepth{
            .price = price,
            .quantity = level.totalQuantity(),
            .orderCount = level.orderCount()
        });
    }

    return depth;
}

} // namespace orderbook
